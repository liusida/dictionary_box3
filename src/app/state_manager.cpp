#include "state_manager.h"
#include "app.h"
#include "core/log.h"

static const char *TAG = "StateManager";

StateManager& StateManager::instance() {
    static StateManager instance;
    return instance;
}

void StateManager::initialize() {
    ESP_LOGI(TAG, "Initializing state manager...");
    
    // Initialize driver health states (following simplified architecture)
    wifiHealthy_ = true;  // Will be updated by checkDriverHealth()
    bleHealthy_ = true;   // Will be updated by checkDriverHealth()
    
    ESP_LOGI(TAG, "State manager initialized - will monitor driver health");
}

void StateManager::shutdown() {
    ESP_LOGI(TAG, "Shutting down state manager...");
    appController_ = nullptr;
    inRecoveryMode_ = false;
}

void StateManager::tick() {
    if (!appController_) {
        return;
    }
    
    // Check service health at regular intervals
    unsigned long currentTime = millis();
    if (currentTime - lastHealthCheck_ >= HEALTH_CHECK_INTERVAL_MS) {
        lastHealthCheck_ = currentTime;
        checkDriverHealth();
    }
    
    // Check if we should return to main from recovery
    checkForRecovery();
}

void StateManager::setAppController(App* app) {
    appController_ = app;
}

AppState StateManager::getCurrentState() const {
    return currentState_;
}

void StateManager::setState(AppState newState) {
    if (currentState_ != newState) {
        ESP_LOGI(TAG, "State transition: %d -> %d", static_cast<int>(currentState_), static_cast<int>(newState));
        currentState_ = newState;
        stateTransitioned_ = true;
    }
}

void StateManager::checkDriverHealth() {
    if (!appController_) {
        return;
    }
    
    // Check WiFi driver health (following simplified architecture)
    bool wifiCurrentlyHealthy = appController_->isSystemReady(); // This checks WiFi connectivity
    if (wifiCurrentlyHealthy != wifiHealthy_) {
        wifiHealthy_ = wifiCurrentlyHealthy;
        if (wifiHealthy_) {
            handleDriverRecovery("WiFi");
        } else {
            handleDriverFailure("WiFi");
        }
    }
    
    // Check BLE driver health (following simplified architecture)
    bool bleCurrentlyHealthy = appController_->isSystemReady(); // This checks BLE connectivity
    if (bleCurrentlyHealthy != bleHealthy_) {
        bleHealthy_ = bleCurrentlyHealthy;
        if (bleHealthy_) {
            handleDriverRecovery("BLE");
        } else {
            handleDriverFailure("BLE");
        }
    }
}

void StateManager::handleDriverFailure(const String& driver) {
    ESP_LOGW(TAG, "Driver failure detected: %s", driver.c_str());
    
    if (driver == "WiFi") {
        wifiFailed_ = true;
        wifiRecovered_ = false;
    } else if (driver == "BLE") {
        bleFailed_ = true;
        bleRecovered_ = false;
    }
    
    // Only transition to recovery screen if we're in main state
    // and enough time has passed since last recovery attempt
    unsigned long currentTime = millis();
    if (appController_ && 
        currentState_ == AppState::MAIN &&
        (currentTime - lastRecoveryTime_ > recoveryCooldown_)) {
        
        transitionToRecoveryScreen(driver);
    }
}

void StateManager::handleDriverRecovery(const String& driver) {
    ESP_LOGI(TAG, "Driver recovery detected: %s", driver.c_str());
    
    if (driver == "WiFi") {
        wifiFailed_ = false;
        wifiRecovered_ = true;
    } else if (driver == "BLE") {
        bleFailed_ = false;
        bleRecovered_ = true;
    }
    
    logRecoveryStatus();
}

void StateManager::transitionToRecoveryScreen(const String& service) {
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

void StateManager::checkForRecovery() {
    if (!inRecoveryMode_ || !appController_) {
        return;
    }
    
    // Check if the service we're recovering has come back online
    bool shouldReturnToMain = false;
    
    if (currentRecoveryService_ == "WiFi" && appController_ && appController_->isSystemReady()) {
        shouldReturnToMain = true;
    } else if (currentRecoveryService_ == "BLE" && appController_ && appController_->isSystemReady()) {
        shouldReturnToMain = true;
    }
    
    if (shouldReturnToMain) {
        ESP_LOGI(TAG, "Service recovered, returning to main screen");
        returnToMain();
    }
}

void StateManager::requestWiFiSettings() {
    if (appController_) {
        appController_->enterWiFiSettingsState();
        inRecoveryMode_ = true;
        currentRecoveryService_ = "WiFi";
    }
}

void StateManager::requestKeyboardSettings() {
    if (appController_) {
        appController_->enterKeyboardSettingsState();
        inRecoveryMode_ = true;
        currentRecoveryService_ = "BLE";
    }
}

void StateManager::returnToMain() {
    if (appController_) {
        appController_->enterMainState();
        inRecoveryMode_ = false;
        currentRecoveryService_ = "";
    }
}

bool StateManager::isInRecoveryMode() const {
    return inRecoveryMode_;
}

String StateManager::getCurrentRecoveryService() const {
    return currentRecoveryService_;
}

bool StateManager::shouldShowRecoveryScreen(const String& service) const {
    // Don't show recovery screen if we're already in recovery mode
    // or if we just showed one recently
    unsigned long currentTime = millis();
    return !inRecoveryMode_ && (currentTime - lastRecoveryTime_ > recoveryCooldown_);
}

void StateManager::logRecoveryStatus() {
    ESP_LOGI(TAG, "Recovery status - WiFi: %s, BLE: %s, Recovery Mode: %s", 
             wifiFailed_ ? "FAILED" : (wifiRecovered_ ? "RECOVERED" : "OK"),
             bleFailed_ ? "FAILED" : (bleRecovered_ ? "RECOVERED" : "OK"),
             inRecoveryMode_ ? "YES" : "NO");
}
