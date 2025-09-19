#pragma once

#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <Preferences.h>
#include <vector>
#include "core/driver_interface.h"

/**
 * WiFiControl Class
 * 
 * This class handles WiFi connection and credential management.
 * UI operations are handled by WiFiSettingsController.
 * 
 * Usage:
 * 1. Create a WiFiControl instance: WiFiControl wifi;
 * 2. Call wifi.begin() - this will try to connect using saved credentials
 * 3. Call wifi.tick() in your main loop for connection monitoring
 * 4. Use wifi.connectToNetwork() for manual connections
 * 
 * Features:
 * - Automatic credential storage in NVS flash
 * - Connection status monitoring
 * - Simple API: wifi.begin(), wifi.tick(), wifi.isConnected(), wifi.getIP()
 */

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
    
    // Connect to a specific network (called by controller)
    bool connectToNetwork(const String& ssid, const String& password);
    
    // Save credentials to NVS
    void saveCredentials(const String& ssid, const String& password);
    
    // Load credentials from NVS
    bool loadCredentials(String& ssid, String& password);
    
    // Clear saved credentials
    void clearCredentials();
    
    // Scan for available networks (returns list of SSIDs)
    std::vector<String> scanNetworks();
    
    // HTTP POST method
    void POST(const String& url, const String& body);

private:
    Preferences preferences;
    bool wifiConnected;
    unsigned long lastConnectionCheck;
    unsigned long lastDisconnectionTime;
    bool wasConnected;
    NetworkClientSecure client;
    HTTPClient https;

    // Try to connect using saved credentials
    bool connectWithSavedCredentials();
};
