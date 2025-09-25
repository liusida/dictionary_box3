#pragma once
#include "ui/ui.h"

// Forward declarations for drivers
class NetworkControl;

/**
 * @brief Simplified WiFi settings screen manager
 * 
 * Handles WiFi settings screen logic and user interactions.
 * Simplified version of the original WiFi settings controller.
 */
class WiFiSettingsScreen {
public:
    WiFiSettingsScreen(NetworkControl& wifiDriver);
    ~WiFiSettingsScreen();
    
    bool initialize();
    void shutdown();
    void tick();
    
    void showWiFiSettingsScreen();
    
private:
    // Driver (following simplified architecture)
    NetworkControl& wifiDriver_;
    
    bool initialized_;
    
    /**
     * @brief Update status icons based on driver state (following simplified architecture)
     */
    void updateStatusIcons();
};
