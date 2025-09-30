#include "network_control.h"
#include "esp_system.h" // for esp_random
#include "esp_wifi.h"
#include "event_publisher.h"
#include "log.h"
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <time.h>
#include <vector>

namespace dict {

static const char *TAG = "WiFi";

// point to the file specified in platformio.ini
// certs/x509_crt_bundle
extern const uint8_t certs_x509_crt_bundle_start[] asm("_binary_certs_x509_crt_bundle_start");
extern const uint8_t certs_x509_crt_bundle_end[] asm("_binary_certs_x509_crt_bundle_end");

NetworkControl::NetworkControl()
    : initialized_(false), connecting_(false), connectStartTime_(0), connectEndTime_(0), wifiConnected(false), lastConnectionCheck(0),
      lastDisconnectionTime(0), wasConnected(false), currentSsid_(""), currentPassword_(""), isOnSettingScreen_(false), scanning_(false), tryingSsid_(""), tryingPassword_("") {}

NetworkControl::~NetworkControl() {}

bool NetworkControl::initialize() { return begin(); }

void NetworkControl::shutdown() {
  // Stop any ongoing HTTP transaction and free resources
  https.end();

  // Prevent automatic reconnects or persistence while tearing down
  WiFi.setAutoReconnect(false);
  WiFi.persistent(false);

  // Disconnect and clear config; stop STA to free driver resources
  WiFi.disconnect(true, true);
  delay(50);
  WiFi.mode(WIFI_OFF);

  // Close preferences namespace if open
  preferences.end();

  // Reset internal state
  initialized_ = false;
  connecting_ = false;
  connectStartTime_ = 0;
  connectEndTime_ = 0;
  wifiConnected = false;
  wasConnected = false;
  pendingSsid_ = "";
  pendingPassword_ = "";
  currentSsid_ = "";
  currentPassword_ = "";
  onConnected_ = {};
  onConnectionFailed_ = {};
  scanning_ = false;
  tryingSsid_ = "";
  tryingPassword_ = "";
  ESP_LOGI(TAG, "WiFi control shutdown complete");
}

bool NetworkControl::isReady() const { return initialized_; }

void NetworkControl::randomizeMACAddress() {
  if (!initialized_) {
    ESP_LOGE(TAG, "WiFi not initialized");
    return;
  }
  uint8_t newMAC[6];
  esp_fill_random(newMAC, sizeof(newMAC));
  // Make sure it’s a unicast, locally administered MAC
  newMAC[0] = (newMAC[0] & 0xFE) | 0x02;
  auto ret = esp_wifi_set_mac(WIFI_IF_STA, newMAC);
  ESP_LOGI(TAG, "esp_wifi_set_mac ret: %d", ret);

  switch (ret) {
  case ESP_OK:
    ESP_LOGIx(TAG, "New MAC: %02X:%02X:%02X:%02X:%02X:%02X\n", newMAC[0], newMAC[1], newMAC[2], newMAC[3], newMAC[4], newMAC[5]);
    break;
  case ESP_ERR_INVALID_ARG:
    ESP_LOGE(TAG, "Invalid argument");
    break;
  case ESP_ERR_WIFI_NOT_INIT:
    ESP_LOGE(TAG, "WiFi not initialized");
    break;
  case ESP_ERR_WIFI_IF:
    ESP_LOGE(TAG, "WiFi not started");
    break;
  case ESP_ERR_WIFI_MAC:
    ESP_LOGE(TAG, "Invalid mac address");
    break;
  case ESP_ERR_WIFI_MODE:
    ESP_LOGE(TAG, "WiFi mode is wrong");
    break;
  default:
    ESP_LOGE(TAG, "Unknown error");
    break;
  }
  return;
}

bool NetworkControl::begin() {
  ESP_LOGI(TAG, "Starting WiFi initialization...");
  WiFi.persistent(false);
  WiFi.setAutoReconnect(false);
  if (!WiFi.mode(WIFI_STA)) {
    ESP_LOGE(TAG, "Failed to set WiFi mode to STA");
    return false;
  }
  client.setCACertBundle(certs_x509_crt_bundle_start, certs_x509_crt_bundle_end - certs_x509_crt_bundle_start);
  WiFi.onEvent([this](arduino_event_id_t event, arduino_event_info_t info) { this->onWiFiEvent(event, info); });
  // Initialize preferences for NVS storage (close first if already open)
  preferences.end();
  if (!preferences.begin("wifi_config", false)) {
    ESP_LOGE(TAG, "Failed to open preferences");
    return false;
  }

  initialized_ = true;
  return true;
}

bool NetworkControl::connectWithSavedCredentials() {
  String ssid, password;

  if (!loadCredentials(ssid, password)) {
    ESP_LOGW(TAG, "No saved credentials found");
    return false;
  }

  if (ssid.length() == 0) {
    ESP_LOGW(TAG, "Empty SSID in saved credentials");
    return false;
  }

  ESP_LOGI(TAG, "Attempting to connect to saved network: %s", ssid.c_str());

  // Use the existing connectToNetwork method with loaded credentials
  setTryingCredentials(ssid, password);
  return connectWithTryingCredentials();
}

void NetworkControl::setTryingCredentials(const String &ssid, const String &password) {
  tryingSsid_ = ssid;
  tryingPassword_ = password;
  return;
}

bool NetworkControl::connectWithTryingCredentials() {
  if (tryingSsid_.length() == 0 || tryingPassword_.length() == 0) {
    ESP_LOGW(TAG, "No trying credentials found");
    return false;
  }
  return connectToNetwork(tryingSsid_, tryingPassword_);
}

void NetworkControl::disconnect() {
  WiFi.disconnect(true, true);
  return;
}

std::vector<String> NetworkControl::scanNetworks() {
  ESP_LOGI(TAG, "Scanning for available networks...");
  if (!initialized_) {
    ESP_LOGE(TAG, "WiFi not initialized");
    return {};
  }
  if (scanning_) {
    ESP_LOGW(TAG, "Scan already in progress");
    return {};
  }
  if (connecting_) {
    ESP_LOGW(TAG, "Cannot scan while connecting");
    return {};
  }
  scanning_ = true;
  ESP_LOGI(TAG, "Setting g_network->scanning to true");

  std::vector<String> networks;

  // Start WiFi in station mode for scanning
  int n = WiFi.scanNetworks();
  ESP_LOGI(TAG, "Found %d networks", n);

  for (int i = 0; i < n; i++) {
    String ssid = WiFi.SSID(i);
    int32_t rssi = WiFi.RSSI(i);
    int32_t encryption = WiFi.encryptionType(i);

    ESP_LOGI(TAG, "%d: %s (%d dBm) %s", i, ssid.c_str(), rssi, (encryption == WIFI_AUTH_OPEN) ? "Open" : "Encrypted");

    networks.push_back(ssid);
  }

  // Clear the scan results
  WiFi.scanDelete();
  scanning_ = false;
  ESP_LOGI(TAG, "Setting g_network->scanning to false");
  return networks;
}

bool NetworkControl::connectToNetwork(const String &ssid, const String &password) {
  if (ssid.length() == 0) {
    ESP_LOGW(TAG, "Empty SSID provided");
    return false;
  }

  ESP_LOGI(TAG, "Starting WiFi.begin to: %s with password: %s", ssid.c_str(), password.c_str());
  connecting_ = true;
  connectStartTime_ = millis();
  connectEndTime_ = 0;
  // Keep credentials in memory and persist only when connected
  pendingSsid_ = ssid;
  pendingPassword_ = password;
  WiFi.begin(ssid.c_str(), password.c_str());
  return true;
}

void NetworkControl::saveCredentials(const String &ssid, const String &password) {
  preferences.end();
  if (!preferences.begin("wifi_config", false)) {
    ESP_LOGE(TAG, "Failed to open preferences for writing");
    return;
  }
  preferences.putString("ssid", ssid);
  preferences.putString("pwd", password);
  preferences.end();
  ESP_LOGI(TAG, "Credentials saved for SSID: %s", ssid.c_str());
}

bool NetworkControl::loadCredentials(String &ssid, String &password) {
  preferences.end();
  if (!preferences.begin("wifi_config", true)) {
    ESP_LOGE(TAG, "Failed to open preferences for reading");
    ssid = "";
    password = "";
    return false;
  }
  ssid = preferences.getString("ssid", "");
  password = preferences.getString("pwd", "");
  preferences.end();

  if (ssid.length() > 0) {
    ESP_LOGI(TAG, "Loaded credentials for SSID: %s", ssid.c_str());
    return true;
  }

  return false;
}

void NetworkControl::clearCredentials() {
  preferences.end();
  if (!preferences.begin("wifi_config", false)) {
    ESP_LOGE(TAG, "Failed to open preferences for clearing");
    return;
  }
  preferences.remove("ssid");
  preferences.remove("pwd");
  preferences.end();
  ESP_LOGI(TAG, "Credentials cleared");
}

bool NetworkControl::hasSavedCredentials() const {
  // Open preferences in read-only mode without altering state
  const_cast<Preferences &>(preferences).end();
  if (!const_cast<Preferences &>(preferences).begin("wifi_config", true)) {
    return false;
  }
  String ssid = const_cast<Preferences &>(preferences).getString("ssid", "");
  const_cast<Preferences &>(preferences).end();
  return ssid.length() > 0;
}

bool NetworkControl::isConnected() { return WiFi.status() == WL_CONNECTED; }

wl_status_t NetworkControl::getStatus() { return WiFi.status(); }

IPAddress NetworkControl::getIP() { return WiFi.localIP(); }

void NetworkControl::tick() {
  unsigned long currentTime = millis();

  // Check connection status every 1 seconds
  if (currentTime - lastConnectionCheck > 1000) {
    lastConnectionCheck = currentTime;

    bool currentlyConnected = (WiFi.status() == WL_CONNECTED);

    // If we just got disconnected
    if (wasConnected && !currentlyConnected) {
      ESP_LOGW(TAG, "Connection lost!");
      wifiConnected = false;
      lastDisconnectionTime = currentTime;
      currentSsid_ = "";
      currentPassword_ = "";
    }
    // If we just got connected
    else if (!wasConnected && currentlyConnected) {
      ESP_LOGI(TAG, "Connection restored! IP: %s", WiFi.localIP().toString().c_str());
      connecting_ = false;
      connectEndTime_ = millis();

      wifiConnected = true;
      lastDisconnectionTime = 0;

      // Set DNS for faster DNS resolution
      WiFi.setDNS(IPAddress(8, 8, 8, 8), IPAddress(114, 114, 114, 114));

      // Set time, HTTPS needs it
      configTime(0, 0, "pool.ntp.org", "time.nist.gov");
      time_t now = 0;
      while (now < 1700000000) { // wait until time is set
        delay(200);
        time(&now);
      }

      // Persist pending credentials if present
      if (pendingSsid_.length() > 0) {
        saveCredentials(pendingSsid_, pendingPassword_);
        currentSsid_ = pendingSsid_;
        currentPassword_ = pendingPassword_;
        pendingSsid_ = "";
        pendingPassword_ = "";
      }
      // fire callback
      if (onConnected_) {
        onConnected_(WiFi.localIP());
      }
    }

    wasConnected = currentlyConnected;
  }
}

void NetworkControl::onWiFiEvent(arduino_event_id_t event, arduino_event_info_t info) {
  switch (event) {
  case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
    ESP_LOGW(TAG, "WiFi disconnected");
    if (connecting_) {
      connecting_ = false;
      connectEndTime_ = millis();

      if (onConnectionFailed_) {
        onConnectionFailed_();
      }
    }
    break;

  case ARDUINO_EVENT_WIFI_STA_CONNECTED:
    ESP_LOGI(TAG, "WiFi connected to AP");
    connecting_ = false;
    connectEndTime_ = millis();
    break;

  case ARDUINO_EVENT_WIFI_STA_GOT_IP:
    ESP_LOGI(TAG, "WiFi got IP address");
    connecting_ = false;
    connectEndTime_ = millis();
    break;

  default:
    ESP_LOGI(TAG, "WiFi event: %d", event);
    break;
  }
}

void NetworkControl::setCACertBundle(WiFiClientSecure &client) {
  client.setCACertBundle(certs_x509_crt_bundle_start, certs_x509_crt_bundle_end - certs_x509_crt_bundle_start);
}

} // namespace dict
