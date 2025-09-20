#pragma once
#include <Arduino.h>
#include "main.h"

/**
 * @brief Runtime state management for service failures and recovery
 * 
 * Handles automatic UI transitions when connectivity services fail or recover
 * during runtime. Provides graceful degradation and user guidance.
 */
class RuntimeStateManager {
public:
    static RuntimeStateManager& instance();
    
    // Lifecycle
    void initialize();
    void shutdown();
    void tick(); // Call in main loop
    
    // State management
    void setAppController(class App* app);
    void handleServiceFailure(const String& service, bool failed);
    void handleServiceRecovery(const String& service, bool recovered);
    
    // Manual triggers
    void requestWiFiSettings();
    void requestKeyboardSettings();
    void returnToMain();
    
    // Status
    bool isInRecoveryMode() const;
    String getCurrentRecoveryService() const;
    
private:
    RuntimeStateManager() = default;
    ~RuntimeStateManager() = default;
    
    // Disable copy constructor and assignment operator
    RuntimeStateManager(const RuntimeStateManager&) = delete;
    RuntimeStateManager& operator=(const RuntimeStateManager&) = delete;
    
    // State tracking
    class App* appController_ = nullptr;
    bool inRecoveryMode_ = false;
    String currentRecoveryService_;
    unsigned long lastRecoveryTime_ = 0;
    unsigned long recoveryCooldown_ = 5000; // 5 seconds between recovery attempts
    
    // Service status tracking
    bool wifiFailed_ = false;
    bool bleFailed_ = false;
    bool wifiRecovered_ = false;
    bool bleRecovered_ = false;
    
    // Internal methods
    void transitionToRecoveryScreen(const String& service);
    void checkForRecovery();
    bool shouldShowRecoveryScreen(const String& service) const;
    void logRecoveryStatus();
};
