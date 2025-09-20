#include <Arduino.h>

#include "core/service_manager.h"
#include "drivers/ble_keyboard.h"
#include "controllers/app.h"
#include "ui/ui.h"
#include "core/log.h"
static const char *TAG = "App";

// Application instance
App app;


void setup() {
    Serial.begin(115200);

    ESP_LOGI(TAG, "=== Dictionary v2 - Two-Tier Architecture ===");
    
    // Tier 1: Initialize core services (essential)
    ESP_LOGI(TAG, "Initializing core services (Tier 1)...");
    if (!ServiceManager::instance().initializeCore()) {
        ESP_LOGE(TAG, "Failed to initialize core services - app cannot continue");
        return;
    }

    ESP_LOGI(TAG, "Initializing UI...");
    // Initialize UI first so splash screen is available
    ui_init();

    ESP_LOGI(TAG, "Initializing application controller...");
    // Initialize application
    if (!app.initialize()) {
        ESP_LOGE(TAG, "Failed to initialize application");
        return;
    }

    ESP_LOGI(TAG, "Entering splash state...");
    // Start in splash state - this will handle connectivity initialization
    app.enterSplashState();

    ESP_LOGI(TAG, "Setup complete - core services ready, splash screen will handle connectivity initialization");
}

void loop() {
    // Use the application
    app.tick();

    delay(5); // Small delay for LVGL
}