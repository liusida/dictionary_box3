#include "connection_monitor.h"
#include "core/service_manager.h"
#include "drivers/wifi_control.h"
#include "drivers/ble_keyboard.h"
#include "core/log.h"

static const char *TAG = "ConnectionMonitor";

ConnectionMonitor& ConnectionMonitor::instance() {
    static ConnectionMonitor instance;
    return instance;
}

void ConnectionMonitor::initialize() {
    if (initialized_) {
        return;
    }
    
    ESP_LOGI(TAG, "Initializing connection monitor...");
    
    // Initialize with current service states
    wifiHealthy_ = ServiceManager::instance().wifi().isConnected();
    bleHealthy_ = ServiceManager::instance().bleKeyboard().isConnected();
    
    if (wifiHealthy_) {
        wifiLastSeen_ = millis();
    }
    if (bleHealthy_) {
        bleLastSeen_ = millis();
    }
    
    initialized_ = true;
    ESP_LOGI(TAG, "Connection monitor initialized - WiFi: %s, BLE: %s", 
             wifiHealthy_ ? "OK" : "FAILED", bleHealthy_ ? "OK" : "FAILED");
}

void ConnectionMonitor::shutdown() {
    if (!initialized_) {
        return;
    }
    
    ESP_LOGI(TAG, "Shutting down connection monitor...");
    initialized_ = false;
}

void ConnectionMonitor::tick() {
    if (!initialized_) {
        return;
    }
    
    unsigned long currentTime = millis();
    
    // Check services at regular intervals
    if (currentTime - lastCheckTime_ >= CHECK_INTERVAL_MS) {
        lastCheckTime_ = currentTime;
        
        checkWiFiHealth();
        checkBLEHealth();
    }
}

void ConnectionMonitor::checkWiFiHealth() {
    bool currentlyConnected = ServiceManager::instance().wifi().isConnected();
    
    if (currentlyConnected) {
        wifiLastSeen_ = millis();
        wifiRecoveryAttempts_ = 0; // Reset attempts on successful connection
        
        if (!wifiHealthy_) {
            // WiFi just recovered
            wifiHealthy_ = true;
            ESP_LOGI(TAG, "WiFi connection recovered");
            publishServiceStatusEvent("WiFi", true);
        }
    } else {
        // WiFi is disconnected
        if (wifiHealthy_) {
            // WiFi just failed
            wifiHealthy_ = false;
            ESP_LOGW(TAG, "WiFi connection lost");
            publishServiceStatusEvent("WiFi", false);
        } else {
            // WiFi has been down for a while, check if we should attempt recovery
            unsigned long timeSinceLastSeen = millis() - wifiLastSeen_;
            if (timeSinceLastSeen > failureThreshold_ && wifiRecoveryAttempts_ < maxRecoveryAttempts_) {
                attemptWiFiRecovery();
            }
        }
    }
}

void ConnectionMonitor::checkBLEHealth() {
    bool currentlyConnected = ServiceManager::instance().bleKeyboard().isConnected();
    
    if (currentlyConnected) {
        bleLastSeen_ = millis();
        bleRecoveryAttempts_ = 0; // Reset attempts on successful connection
        
        if (!bleHealthy_) {
            // BLE just recovered
            bleHealthy_ = true;
            ESP_LOGI(TAG, "BLE connection recovered");
            publishServiceStatusEvent("BLE", true);
        }
    } else {
        // BLE is disconnected
        if (bleHealthy_) {
            // BLE just failed
            bleHealthy_ = false;
            ESP_LOGW(TAG, "BLE connection lost");
            publishServiceStatusEvent("BLE", false);
        } else {
            // BLE has been down for a while, check if we should attempt recovery
            unsigned long timeSinceLastSeen = millis() - bleLastSeen_;
            if (timeSinceLastSeen > failureThreshold_ && bleRecoveryAttempts_ < maxRecoveryAttempts_) {
                attemptBLERecovery();
            }
        }
    }
}

void ConnectionMonitor::publishServiceStatusEvent(const String& service, bool healthy) {
    // Create a custom event for service status changes
    // This will be handled by the app controller to trigger UI transitions
    ESP_LOGI(TAG, "Publishing %s status: %s", service.c_str(), healthy ? "HEALTHY" : "FAILED");
    
    // For now, we'll use the existing event system
    // In a full implementation, you'd create specific service status events
}

void ConnectionMonitor::attemptWiFiRecovery() {
    ESP_LOGI(TAG, "Attempting WiFi recovery (attempt %d/%d)", 
             wifiRecoveryAttempts_ + 1, maxRecoveryAttempts_);
    
    wifiRecoveryAttempts_++;
    
    // Trigger WiFi reconnection
    ServiceManager::instance().wifi().begin();
}

void ConnectionMonitor::attemptBLERecovery() {
    ESP_LOGI(TAG, "Attempting BLE recovery (attempt %d/%d)", 
             bleRecoveryAttempts_ + 1, maxRecoveryAttempts_);
    
    bleRecoveryAttempts_++;
    
    // Trigger BLE reconnection
    ServiceManager::instance().bleKeyboard().begin();
}

bool ConnectionMonitor::isWiFiHealthy() const {
    return wifiHealthy_;
}

bool ConnectionMonitor::isBLEHealthy() const {
    return bleHealthy_;
}

bool ConnectionMonitor::isAnyConnectivityHealthy() const {
    return wifiHealthy_ || bleHealthy_;
}

void ConnectionMonitor::setFailureThreshold(unsigned long thresholdMs) {
    failureThreshold_ = thresholdMs;
}

void ConnectionMonitor::setRecoveryAttempts(int maxAttempts) {
    maxRecoveryAttempts_ = maxAttempts;
}

void ConnectionMonitor::triggerWiFiRecovery() {
    attemptWiFiRecovery();
}

void ConnectionMonitor::triggerBLERecovery() {
    attemptBLERecovery();
}
