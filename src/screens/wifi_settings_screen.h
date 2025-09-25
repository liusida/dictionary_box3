#pragma once
#include "ui/ui.h"

// Forward declarations for drivers
class WiFiControl;

/**
 * @brief Simplified WiFi settings screen manager
 * 
 * Handles WiFi settings screen logic and user interactions.
 * Simplified version of the original WiFi settings controller.
 */
class WiFiSettingsScreen {
public:
    WiFiSettingsScreen(WiFiControl& wifiDriver);
    ~WiFiSettingsScreen();
    
    bool initialize();
    void shutdown();
    void tick();
    
    void showWiFiSettingsScreen();
    
private:
    // Driver (following simplified architecture)
    WiFiControl& wifiDriver_;
    
    bool initialized_;
    
    /**
     * @brief Update status icons based on driver state (following simplified architecture)
     */
    void updateStatusIcons();
};
