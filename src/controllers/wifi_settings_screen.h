#pragma once

#include "ui/ui.h"
#include <Arduino.h>
#include <vector>

/**
 * @brief WiFi Settings screen manager
 * 
 * Handles WiFi configuration and UI interactions
 */
class WiFiSettingsScreen {
public:
    WiFiSettingsScreen();
    ~WiFiSettingsScreen();
    
    bool initialize();
    void shutdown();
    void tick();
    
    void enterWiFiSettingsState();
    void exitWiFiSettingsState();
    
private:
    bool initialized_;

    void loadScreen(lv_obj_t* screen);
    void addObjectToDefaultGroup(lv_obj_t* obj);
    void scanAndPopulateNetworks();
    void resetConnectButton();
    static void connectButtonCallback(lv_event_t * e);
};
