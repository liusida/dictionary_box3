#include <Arduino.h>

#include "app/app.h"
#include "ui/ui.h"
#include "core/log.h"

static const char *TAG = "App";

// Application instance
App app;

void setup() {
    Serial.begin(115200);

    ESP_LOGI(TAG, "=== Dictionary v2 - Simplified Architecture ===");
    
    ESP_LOGI(TAG, "Initializing application...");
    // Initialize application (this will initialize all services including display)
    if (!app.initialize()) {
        ESP_LOGE(TAG, "Failed to initialize application");
        return;
    }

    ESP_LOGI(TAG, "Initializing UI...");
    // Initialize UI after display service is ready
    ui_init();

    ESP_LOGI(TAG, "Entering splash state...");
    // Start in splash state, show screen immediately
    app.enterSplashState();

    app.initializeRemainingComponents();

    // Enter splash state again for the remaining components to be initialized
    app.enterSplashState();

    ESP_LOGI(TAG, "Setup complete - application ready");
}

void loop() {
    // Use the application
    app.tick();

    delay(5); // Small delay for LVGL
}