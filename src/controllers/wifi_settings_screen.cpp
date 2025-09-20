#include "wifi_settings_screen.h"
#include "drivers/lvgl_drive.h"
#include "core/log.h"
#include "drivers/wifi_control.h"
#include "ui/ui.h"
#include "core/service_manager.h"
#include <vector>

static const char* TAG = "WiFiSettingsScreen";

WiFiSettingsScreen::WiFiSettingsScreen() : initialized_(false) {}

WiFiSettingsScreen::~WiFiSettingsScreen() {
    shutdown();
}

bool WiFiSettingsScreen::initialize() {
    ESP_LOGI(TAG, "Initializing WiFi settings controller...");
    initialized_ = true;
    return true;
}

void WiFiSettingsScreen::shutdown() {
    ESP_LOGI(TAG, "Shutting down WiFi settings controller...");
    initialized_ = false;
}

void WiFiSettingsScreen::tick() {
    if (!initialized_) return;
    
    // Handle WiFi settings specific logic
}

void WiFiSettingsScreen::enterWiFiSettingsState() {
    ESP_LOGI(TAG, "Entering WiFi settings state...");
    ui_WIFI_Settings_screen_init();
    loadScreen(ui_WIFI_Settings);
    addObjectToDefaultGroup(ui_InputSSIDs);
    addObjectToDefaultGroup(ui_InputPassword);
    addObjectToDefaultGroup(ui_BtnConnect);
    lv_obj_add_event_cb(ui_BtnConnect, connectButtonCallback, LV_EVENT_CLICKED, nullptr);
    resetConnectButton();
    scanAndPopulateNetworks();
}

void WiFiSettingsScreen::exitWiFiSettingsState() {
    ESP_LOGI(TAG, "Exiting WiFi settings state...");
    ui_WIFI_Settings_screen_destroy();
}

void WiFiSettingsScreen::loadScreen(lv_obj_t* screen) {
    lv_disp_load_scr(screen);
}

void WiFiSettingsScreen::addObjectToDefaultGroup(lv_obj_t* obj) {
    ::addObjectToDefaultGroup(obj);
}

void WiFiSettingsScreen::scanAndPopulateNetworks() {
    // Use the WiFi driver to scan networks
    std::vector<String> networks = ServiceManager::instance().wifi().scanNetworks();
    
    if (networks.empty()) {
        lv_dropdown_set_options(ui_InputSSIDs, "No networks found");
        return;
    }
    
    String options;
    for (size_t i = 0; i < networks.size(); i++) {
        if (i > 0) options += "\n";
        options += networks[i];
    }
    lv_dropdown_set_options(ui_InputSSIDs, options.c_str());
}

void WiFiSettingsScreen::resetConnectButton() {
    if (ui_BtnConnect) {
        lv_obj_clear_state(ui_BtnConnect, LV_STATE_DISABLED);
        lv_label_set_text(lv_obj_get_child(ui_BtnConnect, 0), "Connect");
    }
}

void WiFiSettingsScreen::connectButtonCallback(lv_event_t * e) {
    lv_obj_add_state(ui_BtnConnect, LV_STATE_DISABLED);
    lv_label_set_text(lv_obj_get_child(ui_BtnConnect, 0), "Connecting...");

    uint16_t selected = lv_dropdown_get_selected(ui_InputSSIDs);
    String list = lv_dropdown_get_options(ui_InputSSIDs);
    if (list.length() == 0 || list.indexOf("No networks found") >= 0) {
        lv_label_set_text(lv_obj_get_child(ui_BtnConnect, 0), "No Network!");
        delay(1000);
        if (ui_BtnConnect) {
            lv_obj_clear_state(ui_BtnConnect, LV_STATE_DISABLED);
            lv_label_set_text(lv_obj_get_child(ui_BtnConnect, 0), "Connect");
        }
        return;
    }
    int start = 0;
    for (int i = 0; i < selected; i++) {
        start = list.indexOf('\n', start) + 1;
    }
    int end = list.indexOf('\n', start);
    if (end == -1) end = list.length();
    String ssid = list.substring(start, end);
    String password = lv_textarea_get_text(ui_InputPassword);

    // Use WiFi driver to connect and save credentials
    bool ok = ServiceManager::instance().wifi().connectToNetwork(ssid, password);

    if (ok) {
        lv_label_set_text(lv_obj_get_child(ui_BtnConnect, 0), "Connected!");
        // Let AppController detect connection and transition back
    } else {
        lv_label_set_text(lv_obj_get_child(ui_BtnConnect, 0), "Failed!");
        delay(2000);
        if (ui_BtnConnect) {
            lv_obj_clear_state(ui_BtnConnect, LV_STATE_DISABLED);
            lv_label_set_text(lv_obj_get_child(ui_BtnConnect, 0), "Connect");
        }
    }
}
