#include "ui_status.h"
#include "core_misc/log.h"

namespace dict {

static const char* TAG = "StatusOverlay";

StatusOverlay::StatusOverlay()
    : container_(nullptr), wifiIndicator_(nullptr), bleIndicator_(nullptr), audioIndicator_(nullptr),
      attachedScreen_(nullptr), initialized_(false), visible_(false), indicatorSize_(10),
      animationDuration_(300), wifiState_(WiFiState::None), bleConnected_(false), audioState_(AudioState::None),
      wifiColor_(lv_color_hex(0x0ABF0F)), bleColor_(lv_color_hex(0x4098F2)), audioColor_(lv_color_hex(0xCE45DC)) {
}

StatusOverlay::~StatusOverlay() {
    shutdown();
}

bool StatusOverlay::initialize() {
    if (initialized_) {
        ESP_LOGW(TAG, "StatusOverlay already initialized");
        return true;
    }

    ESP_LOGI(TAG, "Initializing status overlay...");

    // Check if LVGL is initialized by trying to create a simple object
    // This is more reliable than checking DisplayManager state
    null_screen_ = lv_obj_create(NULL);
    if (!null_screen_) {
        ESP_LOGE(TAG, "Failed to create null screen");
        return false;
    }

    // Create main container
    container_ = lv_obj_create(null_screen_); // setting an object's parent to NULL would create a new screen, and attaching won't work, so we make the container a child of the null screen before attaching to any screen.
    if (!container_) {
        ESP_LOGE(TAG, "Failed to create container");
        return false;
    }

    // Set container properties - match SquareLine Studio design
    lv_obj_remove_style_all(container_);
    lv_obj_set_width(container_, 66);  // Match SquareLine Studio width
    lv_obj_set_height(container_, 24); // Match SquareLine Studio height
    lv_obj_set_align(container_, LV_ALIGN_TOP_RIGHT);
    lv_obj_set_flex_flow(container_, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(container_, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_remove_flag(container_, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_pad_left(container_, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(container_, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(container_, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(container_, 5, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Create indicators
    createIndicators();

    initialized_ = true;
    ESP_LOGI(TAG, "Status overlay initialized successfully");
    return true;
}

void StatusOverlay::shutdown() {
    if (!initialized_) {
        return;
    }

    ESP_LOGI(TAG, "Shutting down status overlay...");

    detachFromScreen();

    if (container_) {
        lv_obj_del(container_);
        container_ = nullptr;
    }

    wifiIndicator_ = nullptr;
    bleIndicator_ = nullptr;
    audioIndicator_ = nullptr;
    attachedScreen_ = nullptr;
    initialized_ = false;
    visible_ = false;

    ESP_LOGI(TAG, "Status overlay shutdown complete");
}

void StatusOverlay::tick() {
    if (!initialized_ || !visible_) {
        return;
    }
}

bool StatusOverlay::isReady() const {
    return initialized_;
}

void StatusOverlay::attachToScreen(lv_obj_t* screen) {
    if (!initialized_ || !screen) {
        ESP_LOGW(TAG, "Cannot attach: not initialized or invalid screen");
        return;
    }

    detachFromScreen();
    
    lv_obj_set_parent(container_, screen);
    attachedScreen_ = screen;
    
    ESP_LOGI(TAG, "Status overlay attached to screen");
    ESP_LOGI(TAG, "Container size: %dx%d", lv_obj_get_width(container_), lv_obj_get_height(container_));
    ESP_LOGI(TAG, "Container position: (%d, %d)", lv_obj_get_x(container_), lv_obj_get_y(container_));
}

void StatusOverlay::detachFromScreen() {
    if (attachedScreen_) {
        lv_obj_set_parent(container_, NULL);
        attachedScreen_ = nullptr;
        ESP_LOGI(TAG, "Status overlay detached from screen");
    }
}

void StatusOverlay::show() {
    if (!initialized_) {
        ESP_LOGW(TAG, "Cannot show: not initialized");
        return;
    }

    lv_obj_clear_flag(container_, LV_OBJ_FLAG_HIDDEN);
    visible_ = true;
    ESP_LOGI(TAG, "Status overlay shown");
}

void StatusOverlay::hide() {
    if (!initialized_) {
        return;
    }

    lv_obj_add_flag(container_, LV_OBJ_FLAG_HIDDEN);
    visible_ = false;
    ESP_LOGI(TAG, "Status overlay hidden");
}

bool StatusOverlay::isVisible() const {
    return visible_;
}

void StatusOverlay::updateWiFiStatus(WiFiState state, const String& ssid) {
    wifiState_ = state;
    
    if (initialized_) {
        updateWiFiIndicator();
        const char* stateStr = (state == WiFiState::None) ? "none" : 
                              (state == WiFiState::Ready) ? "ready" : "working";
        ESP_LOGI(TAG, "WiFi status updated: %s (%s)", stateStr, ssid.c_str());
    }
}

void StatusOverlay::updateBLEStatus(bool connected, const String& device) {
    bleConnected_ = connected;
    
    if (initialized_) {
        updateBLEIndicator();
        ESP_LOGI(TAG, "BLE status updated: %s (%s)", connected ? "connected" : "disconnected", device.c_str());
    }
}

void StatusOverlay::updateAudioStatus(AudioState state, const String& track) {
    audioState_ = state;
    
    if (initialized_) {
        updateAudioIndicator();
        const char* stateStr = (state == AudioState::None) ? "none" : 
                              (state == AudioState::Ready) ? "ready" : "working";
        ESP_LOGI(TAG, "Audio status updated: %s (%s)", stateStr, track.c_str());
    }
}

void StatusOverlay::setPosition(lv_align_t align, int32_t x, int32_t y) {
    if (!initialized_) {
        return;
    }

    lv_obj_align(container_, align, x, y);
    ESP_LOGI(TAG, "Status overlay position set to align=%d, x=%d, y=%d", align, x, y);
    ESP_LOGI(TAG, "New position: (%d, %d)", lv_obj_get_x(container_), lv_obj_get_y(container_));
}

void StatusOverlay::setStyle(const lv_style_t* style) {
    if (!initialized_ || !style) {
        return;
    }

    lv_obj_add_style(container_, style, 0);
    ESP_LOGI(TAG, "Status overlay style set");
}

void StatusOverlay::setIndicatorSize(uint8_t size) {
    indicatorSize_ = size;
    
    if (initialized_) {
        // Update existing indicators
        if (wifiIndicator_) {
            lv_obj_set_width(wifiIndicator_, size);
            lv_obj_set_height(wifiIndicator_, size);
        }
        if (bleIndicator_) {
            lv_obj_set_width(bleIndicator_, size);
            lv_obj_set_height(bleIndicator_, size);
        }
        if (audioIndicator_) {
            lv_obj_set_width(audioIndicator_, size);
            lv_obj_set_height(audioIndicator_, size);
        }
        
        ESP_LOGI(TAG, "Indicator size updated to %d", size);
    }
}

void StatusOverlay::setAnimationDuration(uint16_t duration) {
    animationDuration_ = duration;
}

bool StatusOverlay::isAttached() const {
    return attachedScreen_ != nullptr;
}

void StatusOverlay::createIndicators() {
    ESP_LOGI(TAG, "Creating indicators with size %d", indicatorSize_);
    
    // Create WiFi indicator (button style like SquareLine Studio)
    wifiIndicator_ = lv_button_create(container_);
    lv_obj_set_width(wifiIndicator_, indicatorSize_);
    lv_obj_set_height(wifiIndicator_, indicatorSize_);
    lv_obj_set_align(wifiIndicator_, LV_ALIGN_CENTER);
    lv_obj_add_flag(wifiIndicator_, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_remove_flag(wifiIndicator_, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(wifiIndicator_, wifiColor_, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(wifiIndicator_, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    ESP_LOGI(TAG, "WiFi indicator created");

    // Create BLE indicator (button style like SquareLine Studio)
    bleIndicator_ = lv_button_create(container_);
    lv_obj_set_width(bleIndicator_, indicatorSize_);
    lv_obj_set_height(bleIndicator_, indicatorSize_);
    lv_obj_set_align(bleIndicator_, LV_ALIGN_CENTER);
    lv_obj_add_flag(bleIndicator_, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_remove_flag(bleIndicator_, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(bleIndicator_, bleColor_, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(bleIndicator_, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    ESP_LOGI(TAG, "BLE indicator created");

    // Create Audio indicator (button style like SquareLine Studio)
    audioIndicator_ = lv_button_create(container_);
    lv_obj_set_width(audioIndicator_, indicatorSize_);
    lv_obj_set_height(audioIndicator_, indicatorSize_);
    lv_obj_set_align(audioIndicator_, LV_ALIGN_CENTER);
    lv_obj_add_flag(audioIndicator_, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_remove_flag(audioIndicator_, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(audioIndicator_, audioColor_, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(audioIndicator_, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    ESP_LOGI(TAG, "Audio indicator created");

    ESP_LOGI(TAG, "All indicators created successfully");
}

void StatusOverlay::updateWiFiIndicator() {
    if (!wifiIndicator_) return;
    
    lv_color_t color;
    bool isActive = true;
    
    switch (wifiState_) {
        case WiFiState::None:
            color = lv_color_hex(0x333333); // Grey
            isActive = false;
            break;
        case WiFiState::Ready:
            color = wifiColor_; // Green (0x0ABF0F)
            isActive = true;
            break;
        case WiFiState::Working:
            color = lv_color_hex(0x00FF00); // Bright green
            isActive = true;
            break;
    }
    
    applyIndicatorStyle(wifiIndicator_, isActive, color);
}

void StatusOverlay::updateBLEIndicator() {
    if (!bleIndicator_) return;
    
    applyIndicatorStyle(bleIndicator_, bleConnected_, bleColor_);
}

void StatusOverlay::updateAudioIndicator() {
    if (!audioIndicator_) return;
    
    lv_color_t color;
    bool isActive = true;
    
    switch (audioState_) {
        case AudioState::None:
            color = lv_color_hex(0x333333); // Grey
            isActive = false;
            break;
        case AudioState::Ready:
            color = audioColor_; // Purple (0xCE45DC)
            isActive = true;
            break;
        case AudioState::Working:
            color = lv_color_hex(0xFF00FF); // Bright magenta
            isActive = true;
            break;
    }
    
    applyIndicatorStyle(audioIndicator_, isActive, color);
}

void StatusOverlay::applyIndicatorStyle(lv_obj_t* indicator, bool active, lv_color_t activeColor) {
    if (!indicator) return;
    
    lv_color_t color = active ? activeColor : lv_color_hex(0x333333); // Use passed color if active, grey if inactive
    lv_obj_set_style_bg_color(indicator, color, LV_PART_MAIN | LV_STATE_DEFAULT);
    
    // Force immediate UI update for WiFi status changes
    delay(10);
    lv_timer_handler();
    delay(10);
        
    ESP_LOGI(TAG, "Applied %s color to indicator", active ? "active" : "grey");
}

} // namespace dict
