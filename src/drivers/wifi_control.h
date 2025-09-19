#pragma once

#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <Preferences.h>
#include "lvgl.h"
#include "core/driver_interface.h"

/**
 * WiFiControl Class
 * 
 * This class handles WiFi connection with automatic fallback to UI configuration.
 * 
 * Usage:
 * 1. Create a WiFiControl instance: WiFiControl wifi;
 * 2. Call wifi.begin() - this will:
 *    - First try to connect using saved credentials from NVS flash
 *    - If that fails, show the WiFi settings UI for manual configuration
 *    - Save successful credentials for future use
 * 3. Call wifi.tick() in your main loop for automatic disconnection handling
 * 
 * Features:
 * - Automatic credential storage in NVS flash
 * - Network scanning and dropdown population
 * - Connection status monitoring with automatic reconnection
 * - Automatic WiFi settings UI popup on disconnection (after 10 seconds)
 * - Button state management (disabled during connection, re-enabled on failure)
 * - Simple API: wifi.begin(), wifi.tick(), wifi.isConnected(), wifi.getIP()
 */

// Forward declarations for UI elements
extern "C" {
    extern lv_obj_t * ui_WIFI_Settings;
    extern lv_obj_t * ui_InputSSIDs;
    extern lv_obj_t * ui_InputPassword;
    extern lv_obj_t * ui_BtnConnect;
}

class WiFiControl : public DriverInterface {
public:
    WiFiControl();
    ~WiFiControl();
    
    // DriverInterface implementation
    bool initialize() override;
    void shutdown() override;
    void tick() override;
    bool isReady() const override;
    
    // Main entry point - tries saved credentials first, falls back to UI
    bool begin();
    
    // Check if WiFi is connected
    bool isConnected();
    
    // Get current WiFi status
    wl_status_t getStatus();
    
    // Get IP address
    IPAddress getIP();
    
    // Manually trigger WiFi settings UI (useful for user-initiated WiFi setup)
    void showSettingsUI();

private:
    Preferences preferences;
    bool uiInitialized;
    bool wifiConnected;
    unsigned long lastConnectionCheck;
    unsigned long lastDisconnectionTime;
    bool wasConnected;
    NetworkClientSecure client;
    HTTPClient https;

    // Try to connect using saved credentials
    bool connectWithSavedCredentials();
    
    // Show WiFi settings UI and handle user input
    bool showWiFiSettingsUI();
    
    // Close WiFi settings UI and return to main screen
    void closeWiFiSettingsUI();
    
    // Scan for available networks and populate dropdown
    void scanAndPopulateNetworks();
    
    // Connect to selected network
    bool connectToNetwork(const String& ssid, const String& password);
    
    // Save credentials to NVS
    void saveCredentials(const String& ssid, const String& password);
    
    // Load credentials from NVS
    bool loadCredentials(String& ssid, String& password);
    
    // Clear saved credentials
    void clearCredentials();
    
    // Reset connect button to default state
    void resetConnectButton();
    
    // Static callback for connect button
    static void connectButtonCallback(lv_event_t * e);
    
    // Instance pointer for static callback
    static WiFiControl* instance;

    void POST(const String& url, const String& body);
};
