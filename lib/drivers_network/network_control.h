#pragma once

#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <Preferences.h>
#include <vector>
#include <functional>

/**
 * NetworkControl Class
 * 
 * This class handles WiFi connection and credential management.
 * UI operations are handled by WiFiSettingsController.
 * 
 * Usage:
 * 1. Create a NetworkControl instance: NetworkControl wifi;
 * 2. Call wifi.begin() - this will try to connect using saved credentials
 * 3. Call wifi.tick() in your main loop for connection monitoring
 * 4. Use wifi.connectToNetwork() for manual connections
 * 
 * Features:
 * - Automatic credential storage in NVS flash
 * - Connection status monitoring
 * - Simple API: wifi.begin(), wifi.tick(), wifi.isConnected(), wifi.getIP()
 */

class NetworkControl {
public:
    NetworkControl();
    ~NetworkControl();
    
    // Lifecycle
    bool initialize();
    void shutdown();
    void tick();
    bool isReady() const;
    
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

    // Check if valid saved credentials exist in NVS
    bool hasSavedCredentials() const;

    // Connection callbacks
    using ConnectedCallback = std::function<void(const IPAddress&)>;
    using ConnectionFailedCallback = std::function<void()>;
    void setOnConnected(const ConnectedCallback& cb) { onConnected_ = cb; }
    void setOnConnectionFailed(const ConnectionFailedCallback& cb) { onConnectionFailed_ = cb; }
    void randomizeMACAddress();

private:
    Preferences preferences;
    bool wifiConnected;
    unsigned long lastConnectionCheck;
    unsigned long lastDisconnectionTime;
    bool wasConnected;
    // Pending credentials to persist on successful connection
    String pendingSsid_;
    String pendingPassword_;
    // Connection attempt tracking
    bool connectionAttemptInProgress_ = false;
    unsigned long connectionStartTime_ = 0;
    unsigned long connectionTimeoutMs_ = 10000;
    // Callbacks
    ConnectedCallback onConnected_{};
    ConnectionFailedCallback onConnectionFailed_{};
    NetworkClientSecure client;
    HTTPClient https;

    // Try to connect using saved credentials
    bool connectWithSavedCredentials();
};


