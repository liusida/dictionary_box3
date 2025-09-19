#include "wifi_control.h"
#include "ui/ui.h"
#include "drivers/lvgl_drive.h"
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

// Static instance pointer for callbacks
WiFiControl* WiFiControl::instance = nullptr;

WiFiControl::WiFiControl() : uiInitialized(false), wifiConnected(false), 
                            lastConnectionCheck(0), lastDisconnectionTime(0), wasConnected(false) {
    instance = this;
}

WiFiControl::~WiFiControl() {
    if (instance == this) {
        instance = nullptr;
    }
}

bool WiFiControl::initialize() {
    return begin();
}

void WiFiControl::shutdown() {
    // Disconnect WiFi
    WiFi.disconnect();
    ESP_LOGI(TAG, "WiFi control shutdown");
}

bool WiFiControl::isReady() const {
    return const_cast<WiFiControl*>(this)->isConnected();
}

bool WiFiControl::begin() {
    ESP_LOGI(TAG, "Starting WiFi initialization...");
    WiFi.setAutoReconnect(false);
    client.setCACertBundle(certs_x509_crt_bundle_start, certs_x509_crt_bundle_end - certs_x509_crt_bundle_start);
        
    // Initialize preferences for NVS storage
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
    
    ESP_LOGW(TAG, "Failed to connect with saved credentials, showing UI...");
    
    // If that fails, show the WiFi settings UI
    if (showWiFiSettingsUI()) {
        ESP_LOGI(TAG, "Connected via UI configuration");
        wifiConnected = true;
        return true;
    }
    
    ESP_LOGE(TAG, "Failed to connect via UI");
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

bool WiFiControl::showWiFiSettingsUI() {
    ESP_LOGI(TAG, "Initializing WiFi settings UI...");
    
    // Initialize UI if not already done
    if (!uiInitialized) {
        ui_WIFI_Settings_screen_init();
        uiInitialized = true;
    }
    
    // Load the WiFi settings screen
    lv_disp_load_scr(ui_WIFI_Settings);
    
    // Add UI elements to default group for navigation
    addObjectToDefaultGroup(ui_InputSSIDs);
    addObjectToDefaultGroup(ui_InputPassword);
    addObjectToDefaultGroup(ui_BtnConnect);
    
    // Set up connect button callback
    lv_obj_add_event_cb(ui_BtnConnect, connectButtonCallback, LV_EVENT_CLICKED, nullptr);
    
    // Ensure button is in correct initial state
    resetConnectButton();
    
    // Scan for networks and populate dropdown
    scanAndPopulateNetworks();
    
    // Set placeholder text
    lv_textarea_set_placeholder_text(ui_InputPassword, "Enter WiFi password...");
    
    ESP_LOGI(TAG, "WiFi settings UI ready. Waiting for user input...");
    
    // Wait for user to connect (this is a blocking call in the current implementation)
    // In a real application, you might want to handle this differently
    // For now, we'll return false and let the main loop handle the UI interaction
    return false;
}

void WiFiControl::closeWiFiSettingsUI() {
    ESP_LOGI(TAG, "Closing WiFi settings UI and returning to main screen");
    
    // Destroy the WiFi settings screen
    ui_WIFI_Settings_screen_destroy();
    uiInitialized = false;
    
    // Load the main screen (you might want to change this to your actual main screen)
    // For now, we'll just clear the screen
    lv_obj_clean(lv_scr_act());
    
    // You can add code here to load your main application screen
    // For example: lv_disp_load_scr(your_main_screen);
    
    ESP_LOGI(TAG, "WiFi settings UI closed");
}

void WiFiControl::scanAndPopulateNetworks() {
    ESP_LOGI(TAG, "Scanning for available networks...");
    
    // Start WiFi in station mode for scanning
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);
    
    int n = WiFi.scanNetworks();
    ESP_LOGI(TAG, "Found %d networks", n);
    
    if (n == 0) {
        lv_dropdown_set_options(ui_InputSSIDs, "No networks found");
        return;
    }
    
    // Build options string for dropdown
    String options = "";
    for (int i = 0; i < n; i++) {
        String ssid = WiFi.SSID(i);
        int32_t rssi = WiFi.RSSI(i);
        int32_t encryption = WiFi.encryptionType(i);
        
        ESP_LOGI(TAG, "%d: %s (%d dBm) %s", i, ssid.c_str(), rssi, 
                     (encryption == WIFI_AUTH_OPEN) ? "Open" : "Encrypted");
        
        if (options.length() > 0) {
            options += "\n";
        }
        options += ssid;
    }
    
    // Set dropdown options
    lv_dropdown_set_options(ui_InputSSIDs, options.c_str());
    
    // Clear the scan results
    WiFi.scanDelete();
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

void WiFiControl::resetConnectButton() {
    if (ui_BtnConnect) {
        lv_obj_clear_state(ui_BtnConnect, LV_STATE_DISABLED);
        lv_label_set_text(lv_obj_get_child(ui_BtnConnect, 0), "Connect");
    }
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
        
        // Only show UI automatically if it hasn't been shown yet
        // Once UI is shown, don't automatically retry
        if (!uiInitialized && !currentlyConnected && lastDisconnectionTime > 0 && 
            (currentTime - lastDisconnectionTime) > 10000) {
            
            ESP_LOGW(TAG, "WiFi disconnected for too long, showing settings UI...");
            showWiFiSettingsUI();
        }
    }
}

void WiFiControl::showSettingsUI() {
    ESP_LOGI(TAG, "Manually showing WiFi settings UI");
    showWiFiSettingsUI();
}

void WiFiControl::connectButtonCallback(lv_event_t * e) {
    if (!instance) {
        ESP_LOGE(TAG, "No WiFiControl instance available");
        return;
    }
    
    ESP_LOGI(TAG, "Connect button pressed");
    
    // Disable the connect button to prevent multiple clicks
    lv_obj_add_state(ui_BtnConnect, LV_STATE_DISABLED);
    lv_label_set_text(lv_obj_get_child(ui_BtnConnect, 0), "Connecting...");
    
    // Get selected SSID from dropdown
    uint16_t selected = lv_dropdown_get_selected(ui_InputSSIDs);
    String ssid = lv_dropdown_get_options(ui_InputSSIDs);
    
    // Check if a valid network is selected
    if (ssid.length() == 0 || ssid.indexOf("No networks found") >= 0) {
        ESP_LOGW(TAG, "No network selected or no networks available");
        lv_label_set_text(lv_obj_get_child(ui_BtnConnect, 0), "No Network!");
        delay(1000);
        instance->resetConnectButton();
        return;
    }
    
    // Parse the selected SSID from the options string
    int start = 0;
    for (int i = 0; i < selected; i++) {
        start = ssid.indexOf('\n', start) + 1;
    }
    int end = ssid.indexOf('\n', start);
    if (end == -1) end = ssid.length();
    
    String selectedSSID = ssid.substring(start, end);
    
    // Validate SSID
    if (selectedSSID.length() == 0) {
        ESP_LOGW(TAG, "Invalid SSID selected");
        lv_label_set_text(lv_obj_get_child(ui_BtnConnect, 0), "Invalid SSID!");
        delay(1000);
        instance->resetConnectButton();
        return;
    }
    
    // Get password from text area
    String password = lv_textarea_get_text(ui_InputPassword);
    
    ESP_LOGI(TAG, "Attempting to connect to: %s", selectedSSID.c_str());
    
    // Attempt connection
    if (instance->connectToNetwork(selectedSSID, password)) {
        ESP_LOGI(TAG, "Connection successful!");
        
        // Update button text to show success
        lv_label_set_text(lv_obj_get_child(ui_BtnConnect, 0), "Connected!");
        
        // Close the WiFi settings screen and return to main
        instance->closeWiFiSettingsUI();
        
    } else {
        ESP_LOGE(TAG, "Connection failed!");
        
        // Show error message briefly, then reset button
        lv_label_set_text(lv_obj_get_child(ui_BtnConnect, 0), "Failed!");
        
        // Wait a moment to show the error, then reset the button
        delay(2000);
        instance->resetConnectButton();
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
