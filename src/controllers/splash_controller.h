#pragma once

#include "ui/ui.h"
#include <Arduino.h>

/**
 * @brief Controller for the Splash screen
 * 
 * Handles splash screen logic and transitions to main screen
 */
class SplashController {
public:
    SplashController();
    ~SplashController();
    
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
