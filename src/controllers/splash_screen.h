#pragma once

#include "ui/ui.h"
#include <Arduino.h>

/**
 * @brief Splash screen manager
 * 
 * Handles splash screen logic, connectivity initialization, and transitions to main screen
 */
class SplashScreen {
public:
    SplashScreen();
    ~SplashScreen();
    
    bool initialize();
    void shutdown();
    void tick();
    
    void enterSplashState();
    void exitSplashState();
    
    // Connectivity initialization
    void startConnectivityInitialization();
    bool isConnectivityInitialized() const;
    
private:
    bool initialized_;
    unsigned long splashStartTime_;
    bool connectivityStarted_;
    bool connectivityInitialized_;
    static const unsigned long SPLASH_DURATION_MS = 3000; // 3 seconds
    static const unsigned long CONNECTIVITY_TIMEOUT_MS = 10000; // 10 seconds max
    
    void updateSplashProgress();
    void checkConnectivityStatus();
};
