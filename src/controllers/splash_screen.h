#pragma once

#include "ui/ui.h"
#include <Arduino.h>

/**
 * @brief Splash screen manager
 * 
 * Handles splash screen logic and transitions to main screen
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
    
private:
    bool initialized_;
    unsigned long splashStartTime_;
    static const unsigned long SPLASH_DURATION_MS = 3000; // 3 seconds
};
