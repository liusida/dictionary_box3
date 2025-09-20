#include "connectivity_services.h"
#include "drivers/ble_keyboard.h"
#include "drivers/wifi_control.h"
#include "core/log.h"

static const char *TAG = "ConnectivityServices";

ConnectivityServices& ConnectivityServices::instance() {
    static ConnectivityServices instance;
    return instance;
}

bool ConnectivityServices::initialize() {
    if (initialized_) {
        return true;
    }
    
    ESP_LOGI(TAG, "Initializing connectivity services (Tier 2)...");
    
    // Create service instances
    bleKeyboard_ = std::make_unique<BLEKeyboard>();
    wifiControl_ = std::make_unique<WiFiControl>();
    
    // Initialize services - these can fail without breaking the app
    bool bleSuccess = true;
    bool wifiSuccess = true;
    
    try {
        bleKeyboard_->begin();
        ESP_LOGI(TAG, "BLE keyboard initialization started");
    } catch (...) {
        ESP_LOGW(TAG, "BLE keyboard initialization failed");
        bleSuccess = false;
    }
    
    try {
        wifiControl_->begin();
        ESP_LOGI(TAG, "WiFi control initialization started");
    } catch (...) {
        ESP_LOGW(TAG, "WiFi control initialization failed");
        wifiSuccess = false;
    }
    
    initialized_ = true;
    initializationStartTime_ = millis();
    
    ESP_LOGI(TAG, "Connectivity services initialization completed (BLE: %s, WiFi: %s)", 
             bleSuccess ? "OK" : "FAILED", wifiSuccess ? "OK" : "FAILED");
    
    return true; // Always return true since these are optional services
}

void ConnectivityServices::startAsyncInitialization() {
    if (!initialized_) {
        initialize();
    }
}

bool ConnectivityServices::isInitializationComplete() const {
    if (!initialized_) {
        return false;
    }
    
    // Check if timeout has been reached
    if (millis() - initializationStartTime_ > INITIALIZATION_TIMEOUT_MS) {
        return true;
    }
    
    // Consider initialization complete when both services have had time to connect
    // or when timeout is reached
    return (millis() - initializationStartTime_ > 5000); // At least 5 seconds
}

void ConnectivityServices::shutdown() {
    if (!initialized_) {
        return;
    }
    
    ESP_LOGI(TAG, "Shutting down connectivity services...");
    
    // Shutdown services
    if (wifiControl_) {
        wifiControl_->shutdown();
    }
    
    if (bleKeyboard_) {
        bleKeyboard_->shutdown();
    }
    
    // Clear all instances
    bleKeyboard_.reset();
    wifiControl_.reset();
    
    initialized_ = false;
    initializationComplete_ = false;
    ESP_LOGI(TAG, "Connectivity services shutdown complete");
}

bool ConnectivityServices::isReady() const {
    return initialized_ && (isBLEConnected() || isWiFiConnected());
}

bool ConnectivityServices::isBLEConnected() const {
    if (!bleKeyboard_) {
        return false;
    }
    return bleKeyboard_->isConnected();
}

bool ConnectivityServices::isWiFiConnected() const {
    if (!wifiControl_) {
        return false;
    }
    return wifiControl_->isConnected();
}

// Service accessors
BLEKeyboard& ConnectivityServices::bleKeyboard() {
    if (!bleKeyboard_) {
        ESP_LOGE(TAG, "BLEKeyboard not initialized");
        static BLEKeyboard dummy;
        return dummy;
    }
    return *bleKeyboard_;
}

WiFiControl& ConnectivityServices::wifi() {
    if (!wifiControl_) {
        ESP_LOGE(TAG, "WiFiControl not initialized");
        static WiFiControl dummy;
        return dummy;
    }
    return *wifiControl_;
}
