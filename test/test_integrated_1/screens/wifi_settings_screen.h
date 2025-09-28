#pragma once
#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

namespace dict {
class MainScreen;

class WiFiSettingsScreen {
  public:

    void initialize();
    void shutdown();

    void scan();
    void onSelect(const String& ssid);
    void onSubmit();
    void setParent(MainScreen* parent);
    
  private:
    MainScreen* parent_;
    static void scanTask(void *parameter);
    bool scanning_;
    TaskHandle_t scanTaskHandle_;
};

} // namespace dict