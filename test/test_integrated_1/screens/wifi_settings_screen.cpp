#include "wifi_settings_screen.h"
#include "lvgl.h"
#include "main_screen.h"
#include "network_control.h"
#include "ui.h"
namespace dict {

static const char *TAG = "WiFiSettingsScreen";

extern NetworkControl *g_network;

void WiFiSettingsScreen::initialize() {
  g_network->setIsOnSettingScreen(true);
  scanning_ = false;
  scanTaskHandle_ = nullptr;
  ui_WIFI_Settings_screen_init();
  lv_disp_load_scr(ui_WIFI_Settings);
  lv_group_focus_obj(ui_InputPassword);
  if (ui_BtnConnect != NULL) {
    lv_obj_add_event_cb(
        ui_BtnConnect, [](lv_event_t *e) { static_cast<WiFiSettingsScreen *>(lv_event_get_user_data(e))->onSubmit(); }, LV_EVENT_CLICKED, this);
  }
}

void WiFiSettingsScreen::shutdown() {
  parent_ = nullptr;
  if (scanning_) {
    scanning_ = false;
    if (scanTaskHandle_ != nullptr) {
      vTaskDelete(scanTaskHandle_);
      scanTaskHandle_ = nullptr;
    }
  }
  ui_WIFI_Settings_screen_destroy();
  g_network->setIsOnSettingScreen(false);
}

void WiFiSettingsScreen::scanTask(void *parameter) {
  WiFiSettingsScreen *screen = static_cast<WiFiSettingsScreen *>(parameter);

  ESP_LOGI(TAG, "Scan task started");

  while (g_network->isConnecting() || g_network->isScanning()) { // wait until connecting or scanning is finished
    ESP_LOGI(TAG, "Waiting for connecting or scanning to finish");
    delay(500);
  }

  // Perform the actual scan
  auto ssids = g_network->scanNetworks();

  ESP_LOGI(TAG, "Scan task completed, found %d networks", ssids.size());

  // Update UI from the task
  String options = g_network->getCurrentSsid() + "\n";
  for (String ssid : ssids) {
    if (ssid == g_network->getCurrentSsid()) {
      continue;
    }
    options += ssid + "\n";
  }
  options.trim();
  // Set the wifi ssid list
  lv_dropdown_set_options(ui_InputSSIDs, options.c_str());
  // Set dropdown background color to white
  lv_obj_set_style_bg_color(ui_InputSSIDs, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);

  // Clean up
  screen->scanning_ = false;
  screen->scanTaskHandle_ = nullptr;

  ESP_LOGI(TAG, "Scan task finished");
  vTaskDelete(nullptr); // Delete self
}

void WiFiSettingsScreen::scan() {
  if (scanning_) {
    ESP_LOGW(TAG, "Scan already in progress");
    return;
  }
  if (ui_InputSSIDs == NULL) {
    ESP_LOGW(TAG, "WiFi settings UI not initialized, cannot scan");
    return;
  }
  scanning_ = true;
  if (g_network->isConnected()) {
    String options = g_network->getCurrentSsid() + "\nScanning...";
    // Set dropdown background color to gray
    lv_obj_set_style_bg_color(ui_InputSSIDs, lv_color_hex(0xC3C3C3), LV_PART_MAIN | LV_STATE_DEFAULT);    
    lv_dropdown_set_options(ui_InputSSIDs, options.c_str());
    lv_textarea_set_password_mode(ui_InputPassword, false);
    lv_textarea_set_text(ui_InputPassword, g_network->getCurrentPassword().c_str());
    lv_textarea_set_password_mode(ui_InputPassword, true);
  } else {
    lv_dropdown_set_options(ui_InputSSIDs, "Scanning...");
  }

  // Create task for scanning
  BaseType_t result = xTaskCreate(scanTask,        // Task function
                                  "wifi_scan",     // Task name
                                  4096,            // Stack size
                                  this,            // Parameter
                                  1,               // Priority
                                  &scanTaskHandle_ // Task handle
  );

  if (result != pdPASS) {
    ESP_LOGE(TAG, "Failed to create scan task");
    scanning_ = false;
    lv_dropdown_set_options(ui_InputSSIDs, "Scan Failed");
  }
}

void WiFiSettingsScreen::onSelect(const String &ssid) { lv_textarea_set_text(ui_InputPassword, ""); }

void WiFiSettingsScreen::onSubmit() {
  // uint16_t selected_index = lv_dropdown_get_selected(ui_InputSSIDs); // This causes a crash
  char ssid_buffer[64];
  lv_dropdown_get_selected_str(ui_InputSSIDs, ssid_buffer, sizeof(ssid_buffer));
  String ssid = String(ssid_buffer);
  ESP_LOGI(TAG, "As String SSID: %s", ssid.c_str());
  String password = String(lv_textarea_get_text(ui_InputPassword));
  ESP_LOGI(TAG, "As String SSID or password: %s, %s", ssid.c_str(), password.c_str());
  g_network->setTryingCredentials(ssid, password);
  g_network->disconnect();
  parent_->onBackFromWifiSettings();
}

void WiFiSettingsScreen::setParent(MainScreen *parent) { parent_ = parent; }

} // namespace dict