#pragma once
#include "ui/ui.h"

// Forward declarations for drivers
class WiFiControl;
class BLEKeyboard;

/**
 * @brief Simplified splash screen manager
 * 
 * Handles splash screen logic, connectivity initialization, and transitions to main screen.
 * Simplified version of the original splash screen controller.
 */
class SplashScreen {
public:
    SplashScreen(WiFiControl* wifiDriver, BLEKeyboard* bleDriver);
    ~SplashScreen();
    
    bool initialize();
    void shutdown();
    void tick();
    
    void enterSplashState();
    void exitSplashState();
    
    // Connectivity initialization
    void startConnectivityInitialization();
    bool isConnectivityInitialized() const;
    
    // State transition
    bool shouldExitSplash() const;
    
    /**
     * @brief Update status icons based on driver state (following simplified architecture)
     */
    void updateStatusIcons();
    
private:
    // Drivers (following simplified architecture)
    WiFiControl* wifiDriver_;
    BLEKeyboard* bleDriver_;
    
    bool initialized_;
    unsigned long splashStartTime_;
    bool connectivityStarted_;
    bool connectivityInitialized_;
    bool wifiConnectionFailed_;
    bool lastWifiStatus_;
    bool lastBleStatus_;
    
    static const unsigned long SPLASH_DURATION_MS = 3000; // 3 seconds
    static const unsigned long CONNECTIVITY_TIMEOUT_MS = 10000; // 10 seconds max
    
    void updateSplashProgress();
    void checkConnectivityStatus();
};
