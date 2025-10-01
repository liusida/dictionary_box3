#include "wifi_settings_screen.h"
#include "lvgl.h"
#include "main_screen.h"
#include "network_control.h"

namespace dict {

static const char *TAG = "WiFiSettingsScreen";

WiFiSettingsScreen &WiFiSettingsScreen::instance() {
  static WiFiSettingsScreen instance;
  return instance;
}

WiFiSettingsScreen::WiFiSettingsScreen() : initialized_(false), parent_(nullptr) {}

void WiFiSettingsScreen::initialize(MainScreen *parent) {
  parent_ = parent;
  NetworkControl::instance().setIsOnSettingScreen(true);
  scanning_ = false;
  scanTaskHandle_ = nullptr;
  setOptions_ = false;
  options_ = "";
  setTxtStatus_ = false;
  txtStatus_ = "";
  setOptions_ = false;
  options_ = "";
  ui_WIFI_Settings_screen_init();
  lv_disp_load_scr(uiObject());
  lv_group_focus_obj(ui_InputPassword);
  if (ui_BtnConnect != NULL) {
    lv_obj_add_event_cb(
        ui_BtnConnect, [](lv_event_t *e) { static_cast<WiFiSettingsScreen *>(lv_event_get_user_data(e))->onSubmit(); }, LV_EVENT_CLICKED, this);
  }
  initialized_ = true;
}

void WiFiSettingsScreen::shutdown() {
  parent_ = nullptr;
  if (scanning_) {
    scanning_ = false;
    if (scanTaskHandle_ != nullptr) {
      vTaskDelete(scanTaskHandle_);
      NetworkControl::instance().setScanning(false); // when force shutdown, set scanning flag to false
      scanTaskHandle_ = nullptr;
    }
  }
  ui_WIFI_Settings_screen_destroy();
  NetworkControl::instance().setIsOnSettingScreen(false);
  initialized_ = false;
}

void WiFiSettingsScreen::scanTask(void *parameter) {
  WiFiSettingsScreen *screen = static_cast<WiFiSettingsScreen *>(parameter);

  ESP_LOGI(TAG, "Scan task started");
  screen->txtStatus_ = "Preparing to scan...";
  screen->setTxtStatus_ = true;
  while (NetworkControl::instance().isConnecting() || NetworkControl::instance().isScanning()) { // wait until connecting or scanning is finished
    ESP_LOGI(TAG, "Waiting for connecting or scanning to finish. connecting: %d, scanning: %d", NetworkControl::instance().isConnecting(), NetworkControl::instance().isScanning());
    delay(500);
  }
  screen->txtStatus_ = "Scanning WiFi for 5 seconds...";
  screen->setTxtStatus_ = true;
  // Perform the actual scan
  auto ssids = NetworkControl::instance().scanNetworks();

  ESP_LOGI(TAG, "Scan task completed, found %d networks", ssids.size());

  // Update UI from the task
  String options = NetworkControl::instance().getCurrentSsid() + "\n";
  for (String ssid : ssids) {
    if (ssid == NetworkControl::instance().getCurrentSsid()) {
      continue;
    }
    options += ssid + "\n";
  }
  options.trim();
  screen->txtStatus_ = "";
  screen->setTxtStatus_ = true;
  screen->options_ = options;
  screen->setOptions_ = true;

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
  if (NetworkControl::instance().isConnected()) {
    lv_label_set_text(ui_TxtStatus, "Scanning WiFi for 5 seconds...");
    String options = NetworkControl::instance().getCurrentSsid();
    // Set dropdown background color to gray
    lv_obj_set_style_bg_color(ui_InputSSIDs, lv_color_hex(0xC3C3C3), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_dropdown_set_options(ui_InputSSIDs, options.c_str());
    lv_textarea_set_password_mode(ui_InputPassword, false);
    lv_textarea_set_text(ui_InputPassword, NetworkControl::instance().getCurrentPassword().c_str());
    lv_textarea_set_password_mode(ui_InputPassword, true);
  } else {
    lv_dropdown_set_options(ui_InputSSIDs, "");
    lv_obj_set_style_bg_color(ui_InputSSIDs, lv_color_hex(0xC3C3C3), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_label_set_text(ui_TxtStatus, "Scanning WiFi for 5 seconds...");
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
  NetworkControl::instance().setTryingCredentials(ssid, password);
  NetworkControl::instance().disconnect();
  parent_->onBackFromWifiSettings();
}

void WiFiSettingsScreen::tick() {
  if (setTxtStatus_) {
    lv_label_set_text(ui_TxtStatus, txtStatus_.c_str());
    setTxtStatus_ = false;
  }
  if (setOptions_) {
    lv_dropdown_set_options(ui_InputSSIDs, options_.c_str());
    lv_obj_set_style_bg_color(ui_InputSSIDs, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    setOptions_ = false;
  }
}

} // namespace dict