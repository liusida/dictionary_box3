#include "wifi_control.h"
#include "ui/ui.h"
#include "drivers/lvgl_drive.h"
#include <vector>

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

bool WiFiControl::begin() {
    Serial.println("[WiFi] Starting WiFi initialization...");
    
    // Initialize preferences for NVS storage
    if (!preferences.begin("wifi_config", false)) {
        Serial.println("[WiFi] Failed to open preferences");
        return false;
    }
    
    // First, try to connect with saved credentials
    if (connectWithSavedCredentials()) {
        Serial.println("[WiFi] Connected using saved credentials");
        wifiConnected = true;
        wasConnected = true;
        return true;
    }
    
    Serial.println("[WiFi] Failed to connect with saved credentials, showing UI...");
    
    // If that fails, show the WiFi settings UI
    if (showWiFiSettingsUI()) {
        Serial.println("[WiFi] Connected via UI configuration");
        wifiConnected = true;
        return true;
    }
    
    Serial.println("[WiFi] Failed to connect via UI");
    return false;
}

bool WiFiControl::connectWithSavedCredentials() {
    String ssid, password;
    
    if (!loadCredentials(ssid, password)) {
        Serial.println("[WiFi] No saved credentials found");
        return false;
    }
    
    if (ssid.length() == 0) {
        Serial.println("[WiFi] Empty SSID in saved credentials");
        return false;
    }
    
    Serial.printf("[WiFi] Attempting to connect to saved network: %s\n", ssid.c_str());
    
    WiFi.begin(ssid.c_str(), password.c_str());
    
    // Wait for connection with timeout
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        attempts++;
        Serial.print(".");
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.printf("\n[WiFi] Connected! IP address: %s\n", WiFi.localIP().toString().c_str());
        return true;
    } else {
        Serial.println("\n[WiFi] Connection timeout");
        WiFi.disconnect();
        return false;
    }
}

bool WiFiControl::showWiFiSettingsUI() {
    Serial.println("[WiFi] Initializing WiFi settings UI...");
    
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
    
    Serial.println("[WiFi] WiFi settings UI ready. Waiting for user input...");
    
    // Wait for user to connect (this is a blocking call in the current implementation)
    // In a real application, you might want to handle this differently
    // For now, we'll return false and let the main loop handle the UI interaction
    return false;
}

void WiFiControl::closeWiFiSettingsUI() {
    Serial.println("[WiFi] Closing WiFi settings UI and returning to main screen");
    
    // Destroy the WiFi settings screen
    ui_WIFI_Settings_screen_destroy();
    uiInitialized = false;
    
    // Load the main screen (you might want to change this to your actual main screen)
    // For now, we'll just clear the screen
    lv_obj_clean(lv_scr_act());
    
    // You can add code here to load your main application screen
    // For example: lv_disp_load_scr(your_main_screen);
    
    Serial.println("[WiFi] WiFi settings UI closed");
}

void WiFiControl::scanAndPopulateNetworks() {
    Serial.println("[WiFi] Scanning for available networks...");
    
    // Start WiFi in station mode for scanning
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);
    
    int n = WiFi.scanNetworks();
    Serial.printf("[WiFi] Found %d networks\n", n);
    
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
        
        Serial.printf("[WiFi] %d: %s (%d dBm) %s\n", i, ssid.c_str(), rssi, 
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
        Serial.println("[WiFi] Empty SSID provided");
        return false;
    }
    
    Serial.printf("[WiFi] Attempting to connect to: %s\n", ssid.c_str());
    
    WiFi.begin(ssid.c_str(), password.c_str());
    
    // Wait for connection with timeout
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        attempts++;
        Serial.print(".");
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.printf("\n[WiFi] Connected! IP address: %s\n", WiFi.localIP().toString().c_str());
        
        // Save credentials for future use
        saveCredentials(ssid, password);
        
        // Update connection state
        wifiConnected = true;
        wasConnected = true;
        
        return true;
    } else {
        Serial.println("\n[WiFi] Connection failed");
        WiFi.disconnect();
        wifiConnected = false;
        return false;
    }
}

void WiFiControl::saveCredentials(const String& ssid, const String& password) {
    preferences.putString("ssid", ssid);
    preferences.putString("pwd", password);
    Serial.printf("[WiFi] Credentials saved for SSID: %s\n", ssid.c_str());
}

bool WiFiControl::loadCredentials(String& ssid, String& password) {
    ssid = preferences.getString("ssid", "");
    password = preferences.getString("pwd", "");
    
    if (ssid.length() > 0) {
        Serial.printf("[WiFi] Loaded credentials for SSID: %s\n", ssid.c_str());
        return true;
    }
    
    return false;
}

void WiFiControl::clearCredentials() {
    preferences.remove("ssid");
    preferences.remove("pwd");
    Serial.println("[WiFi] Credentials cleared");
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
            Serial.println("[WiFi] Connection lost!");
            wifiConnected = false;
            lastDisconnectionTime = currentTime;
        }
        // If we just got connected
        else if (!wasConnected && currentlyConnected) {
            Serial.printf("[WiFi] Connection restored! IP: %s\n", WiFi.localIP().toString().c_str());
            wifiConnected = true;
            lastDisconnectionTime = 0;
        }
        
        wasConnected = currentlyConnected;
        
        // If disconnected for more than 10 seconds, show WiFi settings UI
        if (!currentlyConnected && lastDisconnectionTime > 0 && 
            (currentTime - lastDisconnectionTime) > 10000) {
            
            Serial.println("[WiFi] WiFi disconnected for too long, showing settings UI...");
            
            // Only show UI if it's not already shown
            if (!uiInitialized) {
                showWiFiSettingsUI();
            }
        }
    }
}

void WiFiControl::showSettingsUI() {
    Serial.println("[WiFi] Manually showing WiFi settings UI");
    showWiFiSettingsUI();
}

void WiFiControl::connectButtonCallback(lv_event_t * e) {
    if (!instance) {
        Serial.println("[WiFi] No WiFiControl instance available");
        return;
    }
    
    Serial.println("[WiFi] Connect button pressed");
    
    // Disable the connect button to prevent multiple clicks
    lv_obj_add_state(ui_BtnConnect, LV_STATE_DISABLED);
    lv_label_set_text(lv_obj_get_child(ui_BtnConnect, 0), "Connecting...");
    
    // Get selected SSID from dropdown
    uint16_t selected = lv_dropdown_get_selected(ui_InputSSIDs);
    String ssid = lv_dropdown_get_options(ui_InputSSIDs);
    
    // Check if a valid network is selected
    if (ssid.length() == 0 || ssid.indexOf("No networks found") >= 0) {
        Serial.println("[WiFi] No network selected or no networks available");
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
        Serial.println("[WiFi] Invalid SSID selected");
        lv_label_set_text(lv_obj_get_child(ui_BtnConnect, 0), "Invalid SSID!");
        delay(1000);
        instance->resetConnectButton();
        return;
    }
    
    // Get password from text area
    String password = lv_textarea_get_text(ui_InputPassword);
    
    Serial.printf("[WiFi] Attempting to connect to: %s\n", selectedSSID.c_str());
    
    // Attempt connection
    if (instance->connectToNetwork(selectedSSID, password)) {
        Serial.println("[WiFi] Connection successful!");
        
        // Update button text to show success
        lv_label_set_text(lv_obj_get_child(ui_BtnConnect, 0), "Connected!");
        
        // Close the WiFi settings screen and return to main
        instance->closeWiFiSettingsUI();
        
    } else {
        Serial.println("[WiFi] Connection failed!");
        
        // Show error message briefly, then reset button
        lv_label_set_text(lv_obj_get_child(ui_BtnConnect, 0), "Failed!");
        
        // Wait a moment to show the error, then reset the button
        delay(2000);
        instance->resetConnectButton();
    }
}
