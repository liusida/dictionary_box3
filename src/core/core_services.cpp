#include "core_services.h"
#include "drivers/audio_manager.h"
#include "drivers/display_manager.h"
#include "input/key_processor.h"
#include "core/log.h"

static const char *TAG = "CoreServices";

CoreServices& CoreServices::instance() {
    static CoreServices instance;
    return instance;
}

bool CoreServices::initialize() {
    if (initialized_) {
        return true;
    }
    
    ESP_LOGI(TAG, "Initializing core services (Tier 1)...");
    
    // Create service instances
    audioManager_ = std::make_unique<AudioManager>();
    displayManager_ = std::make_unique<DisplayManager>();
    keyProcessor_ = std::make_unique<KeyProcessor>();
    
    // Initialize services - all must succeed
    bool success = true;
    
    if (!audioManager_->initialize()) {
        ESP_LOGE(TAG, "Failed to initialize audio manager");
        success = false;
    }
    
    if (!displayManager_->initialize()) {
        ESP_LOGE(TAG, "Failed to initialize display manager");
        success = false;
    }
    
    if (!keyProcessor_->initialize()) {
        ESP_LOGE(TAG, "Failed to initialize key processor");
        success = false;
    }
    
    if (success) {
        initialized_ = true;
        ESP_LOGI(TAG, "Core services initialized successfully");
    } else {
        ESP_LOGE(TAG, "Core services initialization failed");
    }
    
    return success;
}

void CoreServices::shutdown() {
    if (!initialized_) {
        return;
    }
    
    ESP_LOGI(TAG, "Shutting down core services...");
    
    // Shutdown services in reverse order
    if (keyProcessor_) {
        keyProcessor_->shutdown();
    }
    
    if (displayManager_) {
        displayManager_->shutdown();
    }
    
    if (audioManager_) {
        audioManager_->shutdown();
    }
    
    // Clear all instances
    audioManager_.reset();
    displayManager_.reset();
    keyProcessor_.reset();
    
    initialized_ = false;
    ESP_LOGI(TAG, "Core services shutdown complete");
}

bool CoreServices::isReady() const {
    return initialized_;
}

// Service accessors
AudioManager& CoreServices::audio() {
    if (!audioManager_) {
        ESP_LOGE(TAG, "AudioManager not initialized");
        static AudioManager dummy;
        return dummy;
    }
    return *audioManager_;
}

DisplayManager& CoreServices::display() {
    if (!displayManager_) {
        ESP_LOGE(TAG, "DisplayManager not initialized");
        static DisplayManager dummy;
        return dummy;
    }
    return *displayManager_;
}

KeyProcessor& CoreServices::keyProcessor() {
    if (!keyProcessor_) {
        ESP_LOGE(TAG, "KeyProcessor not initialized");
        static KeyProcessor dummy;
        return dummy;
    }
    return *keyProcessor_;
}
