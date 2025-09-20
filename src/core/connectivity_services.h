#pragma once
#include <Arduino.h>
#include <memory>
#include "driver_interface.h"

// Forward declarations
class BLEKeyboard;
class WiFiControl;

/**
 * @brief Connectivity services manager for Tier 2 (optional) services
 * 
 * Manages BLE keyboard and WiFi services that are optional for app
 * functionality. These services can fail to initialize or disconnect
 * without breaking the app.
 */
class ConnectivityServices {
public:
    static ConnectivityServices& instance();
    
    // Service accessors
    BLEKeyboard& bleKeyboard();
    WiFiControl& wifi();
    
    // Lifecycle management
    bool initialize();
    void shutdown();
    
    // Status checking
    bool isReady() const;
    bool isBLEConnected() const;
    bool isWiFiConnected() const;
    
    // Async initialization
    void startAsyncInitialization();
    bool isInitializationComplete() const;
    
private:
    ConnectivityServices() = default;
    ~ConnectivityServices() = default;
    
    // Disable copy constructor and assignment operator
    ConnectivityServices(const ConnectivityServices&) = delete;
    ConnectivityServices& operator=(const ConnectivityServices&) = delete;
    
    // Service instances
    std::unique_ptr<BLEKeyboard> bleKeyboard_;
    std::unique_ptr<WiFiControl> wifiControl_;
    
    // Initialization state
    bool initialized_ = false;
    bool initializationComplete_ = false;
    unsigned long initializationStartTime_ = 0;
    static const unsigned long INITIALIZATION_TIMEOUT_MS = 30000; // 30 seconds
};
