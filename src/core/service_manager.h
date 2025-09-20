#pragma once
#include <Arduino.h>
#include "core_services.h"
#include "connectivity_services.h"

/**
 * @brief Service manager that coordinates between core and connectivity services
 * 
 * Provides a unified interface to access both tiers of services and manages
 * the overall system state. This replaces the old Services class.
 */
class ServiceManager {
public:
    static ServiceManager& instance();
    
    // Core services access (Tier 1)
    AudioManager& audio();
    DisplayManager& display();
    KeyProcessor& keyProcessor();
    
    // Connectivity services access (Tier 2)
    BLEKeyboard& bleKeyboard();
    WiFiControl& wifi();
    
    // Lifecycle management
    bool initializeCore();
    void startConnectivityInitialization();
    void shutdown();
    
    // Status checking
    bool isCoreReady() const;
    bool isConnectivityReady() const;
    bool isSystemReady() const;
    
    // Legacy compatibility
    bool initialize(); // For backward compatibility
    
private:
    ServiceManager();
    ~ServiceManager() = default;
    
    // Disable copy constructor and assignment operator
    ServiceManager(const ServiceManager&) = delete;
    ServiceManager& operator=(const ServiceManager&) = delete;
    
    // Service tier managers
    CoreServices& coreServices_;
    ConnectivityServices& connectivityServices_;
    
    // State tracking
    bool coreInitialized_ = false;
    bool connectivityStarted_ = false;
};
