#include "GT911.h"
#include "TFT_eSPI.h"
#include "lvgl.h"
#include <Arduino.h>

#include "core/services.h"
#include "controllers/app_controller.h"
#include "drivers/wifi_control.h"
#include "drivers/ble_keyboard.h"
#include "drivers/lvgl_drive.h"
#include "ui/ui.h"
#include "utils.h"
#include "controllers/main_screen_control.h"
#include "main.h"
#include "esp_log.h"
static const char *TAG = "App";

// Application controller
AppController appController;

// Legacy state variables for compatibility
AppState currentState = STATE_SPLASH;
bool stateTransitioned = false;
unsigned long lastStateCheck = 0;

// State management functions
void enterSplashState() {
    ESP_LOGI(TAG, "Entering SPLASH state");
    currentState = STATE_SPLASH;
    stateTransitioned = true;
    loadScreen(ui_Splash);
}


bool isSystemReady() {
    // Use the services layer to check system readiness
    bool systemReady = Services::instance().isSystemReady();

    // Only log status every 5 seconds to avoid spam
    unsigned long currentTime = millis();
    if (currentTime - lastStateCheck > 5000) {
        bool wifiReady = Services::instance().wifi().isConnected();
        bool bleReady = Services::instance().bleKeyboard().isConnected();
        ESP_LOGI(TAG, "System status - WiFi: %s, BLE: %s", wifiReady ? "Ready" : "Not Ready", bleReady ? "Ready" : "Not Ready");
        lastStateCheck = currentTime;
    }

    return systemReady;
}

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
    enterSplashState(); // Legacy compatibility

    // Set up BLE keyboard callback to use the new KeyProcessor
    Services::instance().bleKeyboard().setKeyCallback([](char key, uint8_t keyCode, uint8_t modifiers) {
        Services::instance().keyProcessor().sendKeyToLVGL(key, keyCode, modifiers);
    });
}

void loop() {
    // Use the new application controller
    appController.tick();
    
    // Legacy state management for compatibility
    currentState = appController.getCurrentState();
    if (isSystemReady() && currentState == STATE_SPLASH) {
        enterMainState();
    }

    delay(5); // Small delay for LVGL
}