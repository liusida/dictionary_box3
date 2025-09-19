#include <Arduino.h>

#include "core/services.h"
#include "controllers/app_controller.h"
#include "drivers/ble_keyboard.h"
#include "ui/ui.h"
#include "esp_log.h"
static const char *TAG = "App";

// Application controller
AppController appController;


void setup() {
    Serial.begin(115200);

    // Show INFO and above, hide DEBUG/VERBOSE
    esp_log_level_set("*", ESP_LOG_INFO);

    // Initialize all services
    if (!Services::instance().initialize()) {
        ESP_LOGE(TAG, "Failed to initialize services");
        return;
    }

    // Initialize application controller
    if (!appController.initialize()) {
        ESP_LOGE(TAG, "Failed to initialize application controller");
        return;
    }

    // Initialize UI
    ui_init();
    
    // Start in splash state
    appController.enterSplashState();

    // Set up BLE keyboard callback to use the new KeyProcessor
    Services::instance().bleKeyboard().setKeyCallback([](char key, uint8_t keyCode, uint8_t modifiers) {
        Services::instance().keyProcessor().sendKeyToLVGL(key, keyCode, modifiers);
    });
}

void loop() {
    // Use the new application controller
    appController.tick();

    delay(5); // Small delay for LVGL
}