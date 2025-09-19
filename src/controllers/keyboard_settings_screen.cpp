#include "keyboard_settings_screen.h"
#include "drivers/lvgl_drive.h"
#include "core/log.h"
#include "core/services.h"
#include "drivers/ble_keyboard.h"
#include "ui/ui.h"
#include <vector>

static const char* TAG = "KeyboardSettingsScreen";

// Static instance pointer
KeyboardSettingsScreen* KeyboardSettingsScreen::instance_ = nullptr;

KeyboardSettingsScreen::KeyboardSettingsScreen() : initialized_(false) {
    instance_ = this;
}

KeyboardSettingsScreen::~KeyboardSettingsScreen() {
    shutdown();
    if (instance_ == this) {
        instance_ = nullptr;
    }
}

bool KeyboardSettingsScreen::initialize() {
    ESP_LOGI(TAG, "Initializing keyboard settings controller...");
    initialized_ = true;
    return true;
}

void KeyboardSettingsScreen::shutdown() {
    ESP_LOGI(TAG, "Shutting down keyboard settings controller...");
    initialized_ = false;
}

void KeyboardSettingsScreen::tick() {
    if (!initialized_) return;
    
    // Handle keyboard settings specific logic
}

void KeyboardSettingsScreen::enterKeyboardSettingsState() {
    ESP_LOGI(TAG, "Entering keyboard settings state...");
    loadScreen(ui_Keyboard_Settings);
    
    // Set up UI elements
    addObjectToDefaultGroup(ui_InputBLEs);
    addObjectToDefaultGroup(ui_BtnConnectBLE);
    addObjectToDefaultGroup(ui_BtnScan);
    
    // Set up event callbacks
    lv_obj_add_event_cb(ui_BtnScan, scanButtonCallback, LV_EVENT_CLICKED, nullptr);
    lv_obj_add_event_cb(ui_BtnConnectBLE, connectButtonCallback, LV_EVENT_CLICKED, nullptr);
    
    // Initialize dropdown with empty state
    lv_dropdown_set_options(ui_InputBLEs, "No devices found");
    
    // Set button text
    lv_label_set_text(ui_TxtScanBLE, "Scan");
    lv_label_set_text(ui_TxtConnectBLE, "Connect");
}

void KeyboardSettingsScreen::exitKeyboardSettingsState() {
    ESP_LOGI(TAG, "Exiting keyboard settings state...");
    // Return to main screen
}

void KeyboardSettingsScreen::loadScreen(lv_obj_t* screen) {
    lv_disp_load_scr(screen);
}

void KeyboardSettingsScreen::addObjectToDefaultGroup(lv_obj_t* obj) {
    ::addObjectToDefaultGroup(obj);
}

void KeyboardSettingsScreen::scanAndPopulateDevices() {
    ESP_LOGI(TAG, "Scanning for BLE devices...");
    
    // Update button text to show scanning
    lv_label_set_text(ui_TxtScanBLE, "Scanning...");
    lv_obj_add_state(ui_BtnScan, LV_STATE_DISABLED);
    
    // Start BLE scanning
    Services::instance().bleKeyboard().startScan();
    
    // Wait a bit for scan to complete (scan duration is typically 500ms)
    delay(1000);
    
    // Get discovered devices from BLE driver
    std::vector<String> devices = Services::instance().bleKeyboard().getDiscoveredDevices();
    
    if (devices.empty()) {
        lv_dropdown_set_options(ui_InputBLEs, "No devices found");
    } else {
        // Build options string for dropdown
        String options = "";
        for (size_t i = 0; i < devices.size(); i++) {
            if (i > 0) options += "\n";
            options += devices[i];
        }
        lv_dropdown_set_options(ui_InputBLEs, options.c_str());
        ESP_LOGI(TAG, "Found %d devices", devices.size());
    }
    
    // Reset button state
    lv_label_set_text(ui_TxtScanBLE, "Scan");
    lv_obj_clear_state(ui_BtnScan, LV_STATE_DISABLED);
}

void KeyboardSettingsScreen::scanButtonCallback(lv_event_t * e) {
    ESP_LOGI(TAG, "Scan button pressed");
    
    if (instance_) {
        instance_->scanAndPopulateDevices();
    }
}

void KeyboardSettingsScreen::connectButtonCallback(lv_event_t * e) {
    ESP_LOGI(TAG, "Connect button pressed");
    
    // Get selected device from dropdown
    uint16_t selected = lv_dropdown_get_selected(ui_InputBLEs);
    String list = lv_dropdown_get_options(ui_InputBLEs);
    
    if (list.length() == 0 || list.indexOf("No devices found") >= 0) {
        ESP_LOGW(TAG, "No device selected or no devices available");
        return;
    }
    
    // Parse the selected device from the options string
    int start = 0;
    for (int i = 0; i < selected; i++) {
        start = list.indexOf('\n', start) + 1;
    }
    int end = list.indexOf('\n', start);
    if (end == -1) end = list.length();
    String deviceName = list.substring(start, end);
    
    ESP_LOGI(TAG, "Attempting to connect to: %s", deviceName.c_str());
    
    // Update button text to show connecting
    lv_label_set_text(ui_TxtConnectBLE, "Connecting...");
    lv_obj_add_state(ui_BtnConnectBLE, LV_STATE_DISABLED);
    
    // Attempt to connect to the selected device
    bool success = Services::instance().bleKeyboard().connectToDevice(deviceName);
    
    if (success) {
        lv_label_set_text(ui_TxtConnectBLE, "Connected!");
        ESP_LOGI(TAG, "Connection initiated successfully");
        
        // Wait a moment to show success, then the app state machine will handle the transition
        delay(1000);
    } else {
        lv_label_set_text(ui_TxtConnectBLE, "Failed!");
        ESP_LOGW(TAG, "Failed to initiate connection");
        
        // Wait a moment to show error, then reset button
        delay(2000);
        lv_label_set_text(ui_TxtConnectBLE, "Connect");
        lv_obj_clear_state(ui_BtnConnectBLE, LV_STATE_DISABLED);
    }
}
