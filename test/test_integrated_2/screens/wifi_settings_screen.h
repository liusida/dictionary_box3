#pragma once
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <Arduino.h>
#include "ui.h"

namespace dict {
class MainScreen;

class WiFiSettingsScreen {
public:
  // Singleton access
  static WiFiSettingsScreen &instance(); // Get singleton instance

  // Core lifecycle methods
  void initialize(MainScreen *parent);
  void shutdown();
  void tick();
  bool isReady() const { return initialized_; }

  void scan();
  void onSelect(const String &ssid);
  void onSubmit();

  lv_obj_t *uiObject() const { return ui_WIFI_Settings; }

private:
  WiFiSettingsScreen();
  ~WiFiSettingsScreen() = default;
  WiFiSettingsScreen(const WiFiSettingsScreen &) = delete;
  WiFiSettingsScreen &operator=(const WiFiSettingsScreen &) = delete;
  
  bool initialized_;
  MainScreen *parent_;
  static void scanTask(void *parameter);
  bool scanning_;
  TaskHandle_t scanTaskHandle_;
  bool setTxtStatus_;
  String txtStatus_;
  bool setOptions_;
  String options_;
};

} // namespace dict