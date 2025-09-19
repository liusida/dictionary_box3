#include "services.h"
#include "drivers/audio_manager.h"
#include "drivers/ble_keyboard.h"
#include "drivers/wifi_control.h"
#include "drivers/display_manager.h"
#include "input/key_processor.h"
#include "core/log.h"

static const char *TAG = "Services";

Services& Services::instance() {
    static Services instance;
    return instance;
}

bool Services::initialize() {
    if (initialized_) {
        return true;
    }
    
    ESP_LOGI(TAG, "Initializing system services...");
    
    // Create service instances
    audioManager_ = std::make_unique<AudioManager>();
    bleKeyboard_ = std::make_unique<BLEKeyboard>();
    wifiControl_ = std::make_unique<WiFiControl>();
    displayManager_ = std::make_unique<DisplayManager>();
    keyProcessor_ = std::make_unique<KeyProcessor>();
    
    // Initialize services
    bool success = true;
    
    if (!audioManager_->initialize()) {
        ESP_LOGE(TAG, "Failed to initialize audio manager");
        success = false;
    }
    
    if (!displayManager_->initialize()) {
        ESP_LOGE(TAG, "Failed to initialize display manager");
        success = false;
    }
    
    bleKeyboard_->begin();
    wifiControl_->begin();
    keyProcessor_->initialize();
    
    if (success) {
        initialized_ = true;
        ESP_LOGI(TAG, "All services initialized successfully");
    } else {
        ESP_LOGE(TAG, "Some services failed to initialize");
    }
    
    return success;
}

void Services::shutdown() {
    if (!initialized_) {
        return;
    }
    
    ESP_LOGI(TAG, "Shutting down services...");
    
    // Shutdown services in reverse order
    if (keyProcessor_) {
        keyProcessor_->shutdown();
    }
    
    if (wifiControl_) {
        wifiControl_->shutdown();
    }
    
    if (bleKeyboard_) {
        bleKeyboard_->shutdown();
    }
    
    if (displayManager_) {
        displayManager_->shutdown();
    }
    
    if (audioManager_) {
        audioManager_->shutdown();
    }
    
    // Clear all instances
    audioManager_.reset();
    bleKeyboard_.reset();
    wifiControl_.reset();
    displayManager_.reset();
    keyProcessor_.reset();
    
    initialized_ = false;
    ESP_LOGI(TAG, "Services shutdown complete");
}

bool Services::isSystemReady() const {
    if (!initialized_) {
        return false;
    }
    
    return wifiControl_->isConnected() && bleKeyboard_->isConnected();
}

// Service accessors
AudioManager& Services::audio() {
    if (!audioManager_) {
        ESP_LOGE(TAG, "AudioManager not initialized");
        // Return a reference to a static dummy object to avoid crashes
        static AudioManager dummy;
        return dummy;
    }
    return *audioManager_;
}

BLEKeyboard& Services::bleKeyboard() {
    if (!bleKeyboard_) {
        ESP_LOGE(TAG, "BLEKeyboard not initialized");
        static BLEKeyboard dummy;
        return dummy;
    }
    return *bleKeyboard_;
}

WiFiControl& Services::wifi() {
    if (!wifiControl_) {
        ESP_LOGE(TAG, "WiFiControl not initialized");
        static WiFiControl dummy;
        return dummy;
    }
    return *wifiControl_;
}

DisplayManager& Services::display() {
    if (!displayManager_) {
        ESP_LOGE(TAG, "DisplayManager not initialized");
        static DisplayManager dummy;
        return dummy;
    }
    return *displayManager_;
}

KeyProcessor& Services::keyProcessor() {
    if (!keyProcessor_) {
        ESP_LOGE(TAG, "KeyProcessor not initialized");
        static KeyProcessor dummy;
        return dummy;
    }
    return *keyProcessor_;
}
