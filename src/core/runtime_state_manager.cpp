#include "runtime_state_manager.h"
#include "core/connection_monitor.h"
#include "controllers/app.h"
#include "core/log.h"

static const char *TAG = "RuntimeStateManager";

RuntimeStateManager& RuntimeStateManager::instance() {
    static RuntimeStateManager instance;
    return instance;
}

void RuntimeStateManager::initialize() {
    ESP_LOGI(TAG, "Initializing runtime state manager...");
    // Initialization is minimal - most work happens in tick()
}

void RuntimeStateManager::shutdown() {
    ESP_LOGI(TAG, "Shutting down runtime state manager...");
    appController_ = nullptr;
    inRecoveryMode_ = false;
}

void RuntimeStateManager::tick() {
    if (!appController_) {
        return;
    }
    
    // Check current service health
    bool wifiHealthy = ConnectionMonitor::instance().isWiFiHealthy();
    bool bleHealthy = ConnectionMonitor::instance().isBLEHealthy();
    
    // Detect failures
    if (!wifiHealthy && !wifiFailed_) {
        handleServiceFailure("WiFi", true);
    } else if (wifiHealthy && wifiFailed_) {
        handleServiceRecovery("WiFi", true);
    }
    
    if (!bleHealthy && !bleFailed_) {
        handleServiceFailure("BLE", true);
    } else if (bleHealthy && bleFailed_) {
        handleServiceRecovery("BLE", true);
    }
    
    // Check if we should return to main
    checkForRecovery();
}

void RuntimeStateManager::setAppController(class App* app) {
    appController_ = app;
}

void RuntimeStateManager::handleServiceFailure(const String& service, bool failed) {
    if (!failed) return;
    
    ESP_LOGW(TAG, "Service failure detected: %s", service.c_str());
    
    if (service == "WiFi") {
        wifiFailed_ = true;
        wifiRecovered_ = false;
    } else if (service == "BLE") {
        bleFailed_ = true;
        bleRecovered_ = false;
    }
    
    // Only transition to recovery screen if we're in main state
    // and enough time has passed since last recovery attempt
    unsigned long currentTime = millis();
    if (appController_ && 
        appController_->getCurrentState() == STATE_MAIN &&
        (currentTime - lastRecoveryTime_ > recoveryCooldown_)) {
        
        transitionToRecoveryScreen(service);
    }
}

void RuntimeStateManager::handleServiceRecovery(const String& service, bool recovered) {
    if (!recovered) return;
    
    ESP_LOGI(TAG, "Service recovery detected: %s", service.c_str());
    
    if (service == "WiFi") {
        wifiFailed_ = false;
        wifiRecovered_ = true;
    } else if (service == "BLE") {
        bleFailed_ = false;
        bleRecovered_ = true;
    }
    
    logRecoveryStatus();
}

void RuntimeStateManager::transitionToRecoveryScreen(const String& service) {
    if (!appController_) {
        return;
    }
    
    ESP_LOGI(TAG, "Transitioning to recovery screen for %s", service.c_str());
    
    lastRecoveryTime_ = millis();
    inRecoveryMode_ = true;
    currentRecoveryService_ = service;
    
    if (service == "WiFi") {
        appController_->enterWiFiSettingsState();
    } else if (service == "BLE") {
        appController_->enterKeyboardSettingsState();
    }
}

void RuntimeStateManager::checkForRecovery() {
    if (!inRecoveryMode_ || !appController_) {
        return;
    }
    
    // Check if the service we're recovering has come back online
    bool shouldReturnToMain = false;
    
    if (currentRecoveryService_ == "WiFi" && !wifiFailed_) {
        shouldReturnToMain = true;
    } else if (currentRecoveryService_ == "BLE" && !bleFailed_) {
        shouldReturnToMain = true;
    }
    
    if (shouldReturnToMain) {
        ESP_LOGI(TAG, "Service recovered, returning to main screen");
        returnToMain();
    }
}

void RuntimeStateManager::requestWiFiSettings() {
    if (appController_) {
        appController_->enterWiFiSettingsState();
        inRecoveryMode_ = true;
        currentRecoveryService_ = "WiFi";
    }
}

void RuntimeStateManager::requestKeyboardSettings() {
    if (appController_) {
        appController_->enterKeyboardSettingsState();
        inRecoveryMode_ = true;
        currentRecoveryService_ = "BLE";
    }
}

void RuntimeStateManager::returnToMain() {
    if (appController_) {
        appController_->enterMainState();
        inRecoveryMode_ = false;
        currentRecoveryService_ = "";
    }
}

bool RuntimeStateManager::isInRecoveryMode() const {
    return inRecoveryMode_;
}

String RuntimeStateManager::getCurrentRecoveryService() const {
    return currentRecoveryService_;
}

bool RuntimeStateManager::shouldShowRecoveryScreen(const String& service) const {
    // Don't show recovery screen if we're already in recovery mode
    // or if we just showed one recently
    unsigned long currentTime = millis();
    return !inRecoveryMode_ && (currentTime - lastRecoveryTime_ > recoveryCooldown_);
}

void RuntimeStateManager::logRecoveryStatus() {
    ESP_LOGI(TAG, "Recovery status - WiFi: %s, BLE: %s, Recovery Mode: %s", 
             wifiFailed_ ? "FAILED" : (wifiRecovered_ ? "RECOVERED" : "OK"),
             bleFailed_ ? "FAILED" : (bleRecovered_ ? "RECOVERED" : "OK"),
             inRecoveryMode_ ? "YES" : "NO");
}
