#include "wifi_control.h"
#include "core/event_publisher.h"
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <vector>
#include <time.h>
#include "core/log.h"
static const char *TAG = "WiFi";

// point to the file specified in platformio.ini
// certs/x509_crt_bundle
extern const uint8_t certs_x509_crt_bundle_start[] asm("_binary_certs_x509_crt_bundle_start");
extern const uint8_t certs_x509_crt_bundle_end[]   asm("_binary_certs_x509_crt_bundle_end");

WiFiControl::WiFiControl() : wifiConnected(false), 
                            lastConnectionCheck(0), lastDisconnectionTime(0), wasConnected(false) {
}

WiFiControl::~WiFiControl() {
}

bool WiFiControl::initialize() {
    return begin();
}

void WiFiControl::shutdown() {
    // Disconnect WiFi
    WiFi.disconnect();
    // Close preferences
    preferences.end();
    ESP_LOGI(TAG, "WiFi control shutdown");
}

bool WiFiControl::isReady() const {
    return const_cast<WiFiControl*>(this)->isConnected();
}

bool WiFiControl::begin() {
    ESP_LOGI(TAG, "Starting WiFi initialization...");
    WiFi.setAutoReconnect(false);
    client.setCACertBundle(certs_x509_crt_bundle_start, certs_x509_crt_bundle_end - certs_x509_crt_bundle_start);
        
    // Initialize preferences for NVS storage (close first if already open)
    preferences.end();
    if (!preferences.begin("wifi_config", false)) {
        ESP_LOGE(TAG, "Failed to open preferences");
        return false;
    }
    
    // First, try to connect with saved credentials
    if (connectWithSavedCredentials()) {
        ESP_LOGI(TAG, "Connected using saved credentials");
        wifiConnected = true;
        wasConnected = true;
        return true;
    }
    
    ESP_LOGW(TAG, "Failed to connect with saved credentials");
    return false;
}

bool WiFiControl::connectWithSavedCredentials() {
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
    
    WiFi.begin(ssid.c_str(), password.c_str());
    
    // Wait for connection with timeout
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        attempts++;
        // keep silent during retry to avoid spam
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        ESP_LOGI(TAG, "Connected! IP address: %s", WiFi.localIP().toString().c_str());
        
        // Publish WiFi connected event
        EventPublisher::instance().publishWiFiEvent(WiFiEvent::Connected, ssid, WiFi.localIP());
        
        // Set DNS for faster DNS resolution
        WiFi.setDNS(IPAddress(8, 8, 8, 8), IPAddress(114, 114, 114, 114));

        // Set time, HTTPS needs it
        configTime(0, 0, "pool.ntp.org", "time.nist.gov");
        time_t now = 0;
        while (now < 1700000000) {  // wait until time is set
            delay(200);
            time(&now);
        }

        return true;
    } else {
        ESP_LOGE(TAG, "Connection timeout");
        WiFi.disconnect();
        return false;
    }
}

std::vector<String> WiFiControl::scanNetworks() {
    ESP_LOGI(TAG, "Scanning for available networks...");
    
    std::vector<String> networks;
    
    // Start WiFi in station mode for scanning
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);
    
    int n = WiFi.scanNetworks();
    ESP_LOGI(TAG, "Found %d networks", n);
    
    for (int i = 0; i < n; i++) {
        String ssid = WiFi.SSID(i);
        int32_t rssi = WiFi.RSSI(i);
        int32_t encryption = WiFi.encryptionType(i);
        
        ESP_LOGI(TAG, "%d: %s (%d dBm) %s", i, ssid.c_str(), rssi, 
                     (encryption == WIFI_AUTH_OPEN) ? "Open" : "Encrypted");
        
        networks.push_back(ssid);
    }
    
    // Clear the scan results
    WiFi.scanDelete();
    
    return networks;
}

bool WiFiControl::connectToNetwork(const String& ssid, const String& password) {
    if (ssid.length() == 0) {
        ESP_LOGW(TAG, "Empty SSID provided");
        return false;
    }
    
    ESP_LOGI(TAG, "Attempting to connect to: %s", ssid.c_str());
    
    WiFi.begin(ssid.c_str(), password.c_str());
    
    // Wait for connection with timeout
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        attempts++;
        // silent dot printing removed
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        ESP_LOGI(TAG, "Connected! IP address: %s", WiFi.localIP().toString().c_str());
        
        // Save credentials for future use
        saveCredentials(ssid, password);
        
        // Update connection state
        wifiConnected = true;
        wasConnected = true;
        
        return true;
    } else {
        ESP_LOGE(TAG, "Connection failed");
        WiFi.disconnect();
        wifiConnected = false;
        return false;
    }
}

void WiFiControl::saveCredentials(const String& ssid, const String& password) {
    preferences.putString("ssid", ssid);
    preferences.putString("pwd", password);
    ESP_LOGI(TAG, "Credentials saved for SSID: %s", ssid.c_str());
}

bool WiFiControl::loadCredentials(String& ssid, String& password) {
    ssid = preferences.getString("ssid", "");
    password = preferences.getString("pwd", "");
    
    if (ssid.length() > 0) {
        ESP_LOGI(TAG, "Loaded credentials for SSID: %s", ssid.c_str());
        return true;
    }
    
    return false;
}

void WiFiControl::clearCredentials() {
    preferences.remove("ssid");
    preferences.remove("pwd");
    ESP_LOGI(TAG, "Credentials cleared");
}


bool WiFiControl::isConnected() {
    return WiFi.status() == WL_CONNECTED;
}

wl_status_t WiFiControl::getStatus() {
    return WiFi.status();
}

IPAddress WiFiControl::getIP() {
    return WiFi.localIP();
}

void WiFiControl::tick() {
    unsigned long currentTime = millis();
    
    // Check connection status every 5 seconds
    if (currentTime - lastConnectionCheck > 5000) {
        lastConnectionCheck = currentTime;
        
        bool currentlyConnected = (WiFi.status() == WL_CONNECTED);
        
        // If we just got disconnected
        if (wasConnected && !currentlyConnected) {
            ESP_LOGW(TAG, "Connection lost!");
            wifiConnected = false;
            lastDisconnectionTime = currentTime;
        }
        // If we just got connected
        else if (!wasConnected && currentlyConnected) {
            ESP_LOGI(TAG, "Connection restored! IP: %s", WiFi.localIP().toString().c_str());
            wifiConnected = true;
            lastDisconnectionTime = 0;
        }
        
        wasConnected = currentlyConnected;
    }
}



void WiFiControl::POST(const String& url, const String& body) {
    if (WiFi.status() != WL_CONNECTED) {
        ESP_LOGW(TAG, "[HTTPS] Not connected to WiFi");
        return;
    }

    ESP_LOGI(TAG, "[HTTPS] POST %s", url.c_str());

    if (!https.begin(client, url)) {
        ESP_LOGE(TAG, "[HTTPS] begin() failed");
        return;
    }

    https.addHeader("Content-Type", "application/json");
    int httpCode = https.POST(body);

    if (httpCode > 0) {
        ESP_LOGI(TAG, "[HTTPS] code: %d", httpCode);
        String payload = https.getString();
        if (payload.length() > 0) {
            ESP_LOGI(TAG, "%s", payload.c_str());
        }
    } else {
        ESP_LOGE(TAG, "[HTTPS] POST failed, error: %s", https.errorToString(httpCode).c_str());
    }

    https.end();
}
