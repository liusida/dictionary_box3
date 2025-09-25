#include "keyboard_settings_screen.h"
#include "core/log.h"
#include "drivers/ble_keyboard.h"
#include "ui/ui_Keyboard_Settings.h"

static const char *TAG = "KeyboardSettingsScreen";

KeyboardSettingsScreen::KeyboardSettingsScreen(BLEKeyboard& bleDriver) 
    : bleDriver_(bleDriver),
      initialized_(false) {
}

KeyboardSettingsScreen::~KeyboardSettingsScreen() {
    shutdown();
}

bool KeyboardSettingsScreen::initialize() {
    ESP_LOGI(TAG, "Initializing keyboard settings screen controller...");
    initialized_ = true;
    ESP_LOGI(TAG, "Keyboard settings screen controller initialized");
    return true;
}

void KeyboardSettingsScreen::shutdown() {
    ESP_LOGI(TAG, "Shutting down keyboard settings screen controller...");
    initialized_ = false;
}

void KeyboardSettingsScreen::tick() {
    if (!initialized_) {
        return;
    }
    
    // Update UI periodically to refresh device list
    static unsigned long lastUpdate = 0;
    unsigned long now = millis();
    
    if (now - lastUpdate > 1000) { // Update every second
        updateUI();
        updateStatusIcons();
        lastUpdate = now;
    }
}

void KeyboardSettingsScreen::updateStatusIcons() {
    // Update BLE status icon based on driver state (following simplified architecture)
    if (bleDriver_.isConnected()) {
        // TODO: Set BLE icon to blue/connected state
        ESP_LOGD(TAG, "Keyboard Settings: BLE connected - updating status icon");
    } else {
        // TODO: Set BLE icon to red/disconnected state
        ESP_LOGD(TAG, "Keyboard Settings: BLE disconnected - updating status icon");
    }
}

void KeyboardSettingsScreen::showKeyboardSettingsScreen() {
    ESP_LOGI(TAG, "Showing keyboard settings screen");
    
    // Load the keyboard settings screen
    lv_disp_load_scr(ui_Keyboard_Settings);
    ESP_LOGD(TAG, "Keyboard settings screen loaded");
    
    // Set up UI callbacks
    setupUI();
    ESP_LOGD(TAG, "Keyboard settings screen UI setup");

    // Start scanning for BLE devices
    startBLEScan();
    ESP_LOGD(TAG, "Keyboard settings screen BLE scan started");
}

void KeyboardSettingsScreen::setupUI() {
    // Set up scan button callback
    lv_obj_add_event_cb(ui_BtnScan, [](lv_event_t * e) {
        KeyboardSettingsScreen* screen = static_cast<KeyboardSettingsScreen*>(lv_event_get_user_data(e));
        if (screen) {
            screen->onScanButtonPressed();
        }
    }, LV_EVENT_CLICKED, this);
    
    // Set up connect button callback
    lv_obj_add_event_cb(ui_BtnConnectBLE, [](lv_event_t * e) {
        KeyboardSettingsScreen* screen = static_cast<KeyboardSettingsScreen*>(lv_event_get_user_data(e));
        if (screen) {
            screen->onConnectButtonPressed();
        }
    }, LV_EVENT_CLICKED, this);
    
    ESP_LOGD(TAG, "Keyboard settings screen UI setup complete, calling updateUI");
    // Update UI with current status
    updateUI();
}

void KeyboardSettingsScreen::startBLEScan() {
    ESP_LOGI(TAG, "Starting BLE scan for keyboards...");
    
    // Start scanning through BLE driver (following simplified architecture)
    bleDriver_.startScan();
    
    // Update UI to show scanning status
    lv_label_set_text(ui_TxtScanBLE, "Scanning...");
}

void KeyboardSettingsScreen::onScanButtonPressed() {
    ESP_LOGI(TAG, "Scan button pressed");
    startBLEScan();
}

void KeyboardSettingsScreen::onConnectButtonPressed() {
    ESP_LOGI(TAG, "Connect button pressed");
    
    // Get selected device from input field
    String deviceName = lv_textarea_get_text(ui_InputBLEs);
    if (deviceName.length() > 0) {
        ESP_LOGI(TAG, "Attempting to connect to: %s", deviceName.c_str());
        
        // Attempt connection through BLE driver (following simplified architecture)
        bool success = bleDriver_.connectToDevice(deviceName);
        
        if (success) {
            lv_label_set_text(ui_TxtConnectBLE, "Connecting...");
        } else {
            lv_label_set_text(ui_TxtConnectBLE, "Connection failed");
        }
    } else {
        ESP_LOGW(TAG, "No device selected for connection");
        lv_label_set_text(ui_TxtConnectBLE, "Please select a device");
    }
}

void KeyboardSettingsScreen::updateUI() {
    // Update scan button text
    lv_label_set_text(ui_TxtScanBLE, "Scan");
    
    // Update connect button text based on connection status (following simplified architecture)
    if (bleDriver_.isConnected()) {
        lv_label_set_text(ui_TxtConnectBLE, "Connected");
    } else {
        lv_label_set_text(ui_TxtConnectBLE, "Connect");
    }
    
    // Get discovered devices and populate list (following simplified architecture)
    auto devices = bleDriver_.getDiscoveredDevices();
    if (!devices.empty()) {
        String deviceList = "";
        for (size_t i = 0; i < devices.size(); ++i) {
            if (i > 0) deviceList += "\n";
            deviceList += devices[i];
        }
        lv_textarea_set_text(ui_InputBLEs, deviceList.c_str());
    } else {
        // Show message if no devices found
        lv_textarea_set_text(ui_InputBLEs, "No BLE keyboards found\nPress Scan to search");
    }
}
