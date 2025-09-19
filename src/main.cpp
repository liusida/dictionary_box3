#include <Arduino.h>

#include "core/services.h"
#include "controllers/app.h"
#include "input/key_processor.h"
#include "drivers/ble_keyboard.h"
#include "ui/ui.h"
#include "core/log.h"
static const char *TAG = "App";

// Application instance
App app;


void setup() {
    Serial.begin(115200);

    // Initialize all services
    if (!Services::instance().initialize()) {
        ESP_LOGE(TAG, "Failed to initialize services");
        return;
    }

    // Initialize application
    if (!app.initialize()) {
        ESP_LOGE(TAG, "Failed to initialize application");
        return;
    }

    // Initialize UI
    ui_init();
    
    // Start in splash state
    app.enterSplashState();

    // Set up BLE keyboard callback to use the new KeyProcessor
    Services::instance().bleKeyboard().setKeyCallback([](char key, uint8_t keyCode, uint8_t modifiers) {
        Services::instance().keyProcessor().sendKeyToLVGL(key, keyCode, modifiers);
    });
}

void loop() {
    // Use the application
    app.tick();

    delay(5); // Small delay for LVGL
}