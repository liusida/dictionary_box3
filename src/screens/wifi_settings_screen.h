#pragma once
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <Arduino.h>

namespace dict {
class MainScreen;

class WiFiSettingsScreen {
public:
  void initialize();
  void shutdown();
  void tick();

  void scan();
  void onSelect(const String &ssid);
  void onSubmit();
  void setParent(MainScreen *parent);

private:
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