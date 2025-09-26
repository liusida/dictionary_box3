#include "ui_status.h"
#include "log.h"

namespace dict {

static const char* TAG = "StatusOverlay";

StatusOverlay::StatusOverlay()
    : container_(nullptr), wifiIndicator_(nullptr), bleIndicator_(nullptr), audioIndicator_(nullptr),
      attachedScreen_(nullptr), initialized_(false), visible_(false), indicatorSize_(20),
      animationDuration_(300), wifiConnected_(false), bleConnected_(false), audioPlaying_(false) {
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

    // Create main container
    container_ = lv_obj_create(NULL);
    if (!container_) {
        ESP_LOGE(TAG, "Failed to create container");
        return false;
    }

    // Set container properties
    lv_obj_set_size(container_, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_style_pad_all(container_, 5, 0);
    lv_obj_set_style_bg_opa(container_, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(container_, 0, 0);

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

    // Update indicator animations if needed
    // This can be extended for more complex animations
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

void StatusOverlay::updateWiFiStatus(bool connected, const String& ssid) {
    wifiConnected_ = connected;
    wifiSSID_ = ssid;
    
    if (initialized_) {
        updateWiFiIndicator();
        ESP_LOGI(TAG, "WiFi status updated: %s (%s)", connected ? "connected" : "disconnected", ssid.c_str());
    }
}

void StatusOverlay::updateBLEStatus(bool connected, const String& device) {
    bleConnected_ = connected;
    bleDevice_ = device;
    
    if (initialized_) {
        updateBLEIndicator();
        ESP_LOGI(TAG, "BLE status updated: %s (%s)", connected ? "connected" : "disconnected", device.c_str());
    }
}

void StatusOverlay::updateAudioStatus(bool playing, const String& track) {
    audioPlaying_ = playing;
    audioTrack_ = track;
    
    if (initialized_) {
        updateAudioIndicator();
        ESP_LOGI(TAG, "Audio status updated: %s (%s)", playing ? "playing" : "stopped", track.c_str());
    }
}

void StatusOverlay::setPosition(lv_align_t align, int32_t x, int32_t y) {
    if (!initialized_) {
        return;
    }

    lv_obj_align(container_, align, x, y);
    ESP_LOGI(TAG, "Status overlay position set");
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
        if (wifiIndicator_) lv_obj_set_size(wifiIndicator_, size, size);
        if (bleIndicator_) lv_obj_set_size(bleIndicator_, size, size);
        if (audioIndicator_) lv_obj_set_size(audioIndicator_, size, size);
    }
}

void StatusOverlay::setAnimationDuration(uint16_t duration) {
    animationDuration_ = duration;
}

bool StatusOverlay::isAttached() const {
    return attachedScreen_ != nullptr;
}

void StatusOverlay::createIndicators() {
    // Create WiFi indicator
    wifiIndicator_ = lv_obj_create(container_);
    lv_obj_set_size(wifiIndicator_, indicatorSize_, indicatorSize_);
    lv_obj_set_style_bg_color(wifiIndicator_, lv_color_hex(0xFF0000), 0); // Red by default
    lv_obj_set_style_border_width(wifiIndicator_, 0, 0);
    lv_obj_align(wifiIndicator_, LV_ALIGN_TOP_LEFT, 0, 0);

    // Create BLE indicator
    bleIndicator_ = lv_obj_create(container_);
    lv_obj_set_size(bleIndicator_, indicatorSize_, indicatorSize_);
    lv_obj_set_style_bg_color(bleIndicator_, lv_color_hex(0xFF0000), 0); // Red by default
    lv_obj_set_style_border_width(bleIndicator_, 0, 0);
    lv_obj_align_to(bleIndicator_, wifiIndicator_, LV_ALIGN_OUT_RIGHT_MID, 5, 0);

    // Create Audio indicator
    audioIndicator_ = lv_obj_create(container_);
    lv_obj_set_size(audioIndicator_, indicatorSize_, indicatorSize_);
    lv_obj_set_style_bg_color(audioIndicator_, lv_color_hex(0xFF0000), 0); // Red by default
    lv_obj_set_style_border_width(audioIndicator_, 0, 0);
    lv_obj_align_to(audioIndicator_, bleIndicator_, LV_ALIGN_OUT_RIGHT_MID, 5, 0);

    // Add click callbacks
    lv_obj_add_event_cb(wifiIndicator_, indicatorClickCallback, LV_EVENT_CLICKED, this);
    lv_obj_add_event_cb(bleIndicator_, indicatorClickCallback, LV_EVENT_CLICKED, this);
    lv_obj_add_event_cb(audioIndicator_, indicatorClickCallback, LV_EVENT_CLICKED, this);
}

void StatusOverlay::updateWiFiIndicator() {
    if (!wifiIndicator_) return;
    
    applyIndicatorStyle(wifiIndicator_, wifiConnected_);
}

void StatusOverlay::updateBLEIndicator() {
    if (!bleIndicator_) return;
    
    applyIndicatorStyle(bleIndicator_, bleConnected_);
}

void StatusOverlay::updateAudioIndicator() {
    if (!audioIndicator_) return;
    
    applyIndicatorStyle(audioIndicator_, audioPlaying_);
}

void StatusOverlay::applyIndicatorStyle(lv_obj_t* indicator, bool active) {
    if (!indicator) return;
    
    lv_color_t color = active ? lv_color_hex(0x00FF00) : lv_color_hex(0xFF0000); // Green if active, red if inactive
    lv_obj_set_style_bg_color(indicator, color, 0);
}

void StatusOverlay::indicatorClickCallback(lv_event_t* e) {
    StatusOverlay* overlay = static_cast<StatusOverlay*>(lv_event_get_user_data(e));
    lv_obj_t* indicator = static_cast<lv_obj_t*>(lv_event_get_target(e));
    
    if (!overlay) return;
    
    // Determine which indicator was clicked
    if (indicator == overlay->wifiIndicator_) {
        ESP_LOGI(TAG, "WiFi indicator clicked - SSID: %s", overlay->wifiSSID_.c_str());
    } else if (indicator == overlay->bleIndicator_) {
        ESP_LOGI(TAG, "BLE indicator clicked - Device: %s", overlay->bleDevice_.c_str());
    } else if (indicator == overlay->audioIndicator_) {
        ESP_LOGI(TAG, "Audio indicator clicked - Track: %s", overlay->audioTrack_.c_str());
    }
}

} // namespace dict
