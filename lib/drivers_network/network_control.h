#pragma once

#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <Preferences.h>
#include <vector>
#include <functional>

namespace dict {

class NetworkControl {
public:
    // Constructor/Destructor
    NetworkControl();
    ~NetworkControl();
    
    // Core lifecycle methods
    bool initialize(); // Initialize network control (calls begin())
    void shutdown(); // Clean shutdown of WiFi and clear credentials
    void tick(); // Monitor connection status and handle reconnection
    bool isReady() const; // Check if network control is ready
    
    // Main entry point - tries saved credentials first, falls back to UI
    bool begin(); // Start WiFi connection using saved credentials or show UI
    
    // Connection management methods
    bool isConnected(); // Check if WiFi is currently connected
    wl_status_t getStatus(); // Get current WiFi connection status
    IPAddress getIP(); // Get current IP address
    bool connectToNetwork(const String& ssid, const String& password); // Connect to specific network
    
    // Credential management methods
    void saveCredentials(const String& ssid, const String& password); // Save WiFi credentials to NVS
    bool loadCredentials(String& ssid, String& password); // Load saved credentials from NVS
    void clearCredentials(); // Clear saved credentials from NVS
    bool hasSavedCredentials() const; // Check if valid credentials exist in NVS
    
    // Network scanning methods
    std::vector<String> scanNetworks(); // Scan for available WiFi networks

    // Connection callbacks
    using ConnectedCallback = std::function<void(const IPAddress&)>;
    using ConnectionFailedCallback = std::function<void()>;
    void setOnConnected(const ConnectedCallback& cb) { onConnected_ = cb; } // Set callback for successful connection
    void setOnConnectionFailed(const ConnectionFailedCallback& cb) { onConnectionFailed_ = cb; } // Set callback for connection failure
    void randomizeMACAddress(); // Randomize WiFi MAC address for privacy

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

} // namespace dict


