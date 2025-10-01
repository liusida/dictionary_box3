#pragma once
#include "common.h"
#include <HTTPClient.h>
#include <Preferences.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>

namespace dict {

class NetworkControl {
public:
  // Constructor/Destructor
  NetworkControl();
  ~NetworkControl();

  // Core lifecycle methods
  bool initialize();    // Initialize network control (calls begin())
  void shutdown();      // Clean shutdown of WiFi and clear credentials
  void tick();          // Monitor connection status and handle reconnection
  bool isReady() const; // Check if network control is ready

  // Main entry point - tries saved credentials first, falls back to UI
  bool begin(); // Initialize network control
  bool isConnecting() const { return connecting_; }
  uint32_t getConnectStartTime() const { return connectStartTime_; } // Get start time of current connection attempt
  uint32_t getConnectEndTime() const { return connectEndTime_; }     // Get end time of current connection attempt

  // Connection management methods
  bool isConnected();      // Check if WiFi is currently connected
  wl_status_t getStatus(); // Get current WiFi connection status
  IPAddress getIP();       // Get current IP address
  bool connectWithTryingCredentials();
  bool connectWithSavedCredentials();
  bool connectToNetwork(const String &ssid, const String &password); // Connect to specific network
  void disconnect();
  WiFiClientSecure &getClient() { return client; } // Get the HTTPS client
  void randomizeMACAddress();                      // Randomize WiFi MAC address for privacy
  void setCACertBundle(WiFiClientSecure &client);

  // Credential management methods
  void saveCredentials(const String &ssid, const String &password); // Save WiFi credentials to NVS
  bool loadCredentials(String &ssid, String &password);             // Load saved credentials from NVS
  void clearCredentials();                                          // Clear saved credentials from NVS
  bool hasSavedCredentials() const;                                 // Check if valid credentials exist in NVS
  String getCurrentSsid() const { return currentSsid_; }
  String getCurrentPassword() const { return currentPassword_; }

  // Network scanning methods
  std::vector<String> scanNetworks(); // Scan for available WiFi networks
  bool isScanning() const { return scanning_; }
  void setScanning(bool scanning) { scanning_ = scanning; }

  // Connection callbacks
  using ConnectedCallback = std::function<void(const IPAddress &)>;
  using ConnectionFailedCallback = std::function<void()>;
  void setOnConnected(const ConnectedCallback &cb) { onConnected_ = cb; }                      // Set callback for successful connection
  void setOnConnectionFailed(const ConnectionFailedCallback &cb) { onConnectionFailed_ = cb; } // Set callback for connection failure
  void setTryingCredentials(const String &ssid, const String &password);

  // Event callbacks
  void onWiFiEvent(arduino_event_id_t event, arduino_event_info_t info);

  void setIsOnSettingScreen(bool isOnSettingScreen) { isOnSettingScreen_ = isOnSettingScreen; }
  bool isOnSettingScreen() const { return isOnSettingScreen_; }

private:
  Preferences preferences;
  bool initialized_;
  bool connecting_;
  uint32_t connectStartTime_;
  uint32_t connectEndTime_;
  bool scanning_;
  bool wifiConnected;
  unsigned long lastConnectionCheck;
  unsigned long lastDisconnectionTime;
  bool wasConnected;
  // Pending credentials to persist on successful connection
  String tryingSsid_;
  String tryingPassword_;
  String pendingSsid_;
  String pendingPassword_;
  String currentSsid_;
  String currentPassword_;

  // Callbacks
  ConnectedCallback onConnected_{};
  ConnectionFailedCallback onConnectionFailed_{};
  NetworkClientSecure client;
  HTTPClient https;

  bool isOnSettingScreen_;
};

} // namespace dict
