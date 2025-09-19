#include "display_manager.h"
#include "lvgl_drive.h"
#include "utils.h"
#include "esp_log.h"

static const char *TAG = "DisplayManager";

DisplayManager::DisplayManager() 
    : displayInitialized_(false), touchInitialized_(false), lvglInitialized_(false) {
}

DisplayManager::~DisplayManager() {
    shutdown();
}

bool DisplayManager::initialize() {
    if (displayInitialized_) {
        return true;
    }
    
    ESP_LOGI(TAG, "Initializing display manager...");
    
    // Reset display and turn on backlight
    resetDisplay();
    
    // Initialize TFT display
    tft_.init();
    tft_.setRotation(3);
    displayInitialized_ = true;
    
    // Initialize touch controller
    if (!initTouch()) {
        ESP_LOGE(TAG, "Failed to initialize touch controller");
        return false;
    }
    
    // Initialize LVGL
    initLVGL();
    
    ESP_LOGI(TAG, "Display manager initialized successfully");
    return true;
}

void DisplayManager::shutdown() {
    if (!displayInitialized_) {
        return;
    }
    
    ESP_LOGI(TAG, "Shutting down display manager...");
    
    // LVGL cleanup would go here if needed
    lvglInitialized_ = false;
    touchInitialized_ = false;
    displayInitialized_ = false;
    
    ESP_LOGI(TAG, "Display manager shutdown complete");
}

void DisplayManager::tick() {
    if (isReady()) {
        handleLVGLTasks();
    }
}

bool DisplayManager::isReady() const {
    return displayInitialized_ && touchInitialized_ && lvglInitialized_;
}

void DisplayManager::resetDisplay() {
    manualResetDisplay();
}

void DisplayManager::setBacklight(bool on) {
    digitalWrite(TFT_BL, on ? HIGH : LOW);
}

bool DisplayManager::initTouch() {
    if (touchInitialized_) {
        return true;
    }
    
    touchInitialized_ = ::initTouch(touch_);
    return touchInitialized_;
}

void DisplayManager::handleTouch() {
    // Touch handling is done in the LVGL callback
    // This method is here for future expansion
}

void DisplayManager::initLVGL() {
    if (lvglInitialized_) {
        return;
    }
    
    // Initialize LVGL
    lv_init();
    initLVGLDisplay(tft_, touch_);
    lvglInitialized_ = true;
}

void DisplayManager::handleLVGLTasks() {
    ::handleLVGLTasks();
}
