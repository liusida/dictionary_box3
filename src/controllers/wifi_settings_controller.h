#pragma once

#include "ui/ui.h"
#include <Arduino.h>

/**
 * @brief Controller for the WiFi Settings screen
 * 
 * Handles WiFi configuration and UI interactions
 */
class WiFiSettingsController {
public:
    WiFiSettingsController();
    ~WiFiSettingsController();
    
    bool initialize();
    void shutdown();
    void tick();
    
    void enterWiFiSettingsState();
    void exitWiFiSettingsState();
    
private:
    bool initialized_;
};
