#include "GT911.h"
#include "TFT_eSPI.h"
#include "lvgl.h"
#include <Arduino.h>

#include "drivers/drivers.h"
#include "ui/ui.h"
#include "utils.h"
#include "main_screen_control.h"
#include "main.h"

// Global objects
TFT_eSPI tft = TFT_eSPI();
GT911 touch = GT911();
BLEKeyboard bleKeyboard;
WiFiControl wifi;

// State management
AppState currentState = STATE_SPLASH;
bool stateTransitioned = false;
unsigned long lastStateCheck = 0;

// State management functions
void enterSplashState() {
    Serial.println("[App] Entering SPLASH state");
    currentState = STATE_SPLASH;
    stateTransitioned = true;
    loadScreen(ui_Splash);
}


bool isSystemReady() {
    // Check if both WiFi and BLE keyboard are ready
    bool wifiReady = wifi.isConnected();
    bool bleReady = bleKeyboard.isConnected();

    // Only log status every 5 seconds to avoid spam
    unsigned long currentTime = millis();
    if (currentTime - lastStateCheck > 5000) {
        Serial.printf("[App] System status - WiFi: %s, BLE: %s\n", wifiReady ? "Ready" : "Not Ready", bleReady ? "Ready" : "Not Ready");
        lastStateCheck = currentTime;
    }

    return wifiReady && bleReady;
}

void setup() {
    Serial.begin(115200);

    // Reset display and turn on backlight
    manualResetDisplay();

    // Initialize TFT display
    tft.init();
    tft.setRotation(3);

    // Initialize touch controller
    initTouch(touch);

    // Initialize LVGL
    lv_init();
    initLVGLDisplay(tft, touch);

    // Initialize UI
    ui_init();
    // Start in splash state
    enterSplashState();

    // Initialize BLE keyboard functionality
    bleKeyboard.setPowerLevel(-20);
    bleKeyboard.begin();

    // Set callback to connect BLE keyboard to LVGL key processing
    bleKeyboard.setKeyCallback(sendKeyToLVGL);

    // Initialize WiFi - this will try saved credentials first, then show UI if needed
    wifi.begin();
}

void loop() {
    // Handle LVGL tasks
    handleLVGLTasks();

    bleKeyboard.tick();
    wifi.tick();

    // Process queued keyboard events (safe for LVGL operations)
    processQueuedKeys();

    // State machine logic
    switch (currentState) {
    case STATE_SPLASH:
        // Check if system is ready to transition to main
        if (isSystemReady()) {
            enterMainState();
        }
        break;

    case STATE_MAIN:
        // just stay here
        break;
    }

    delay(5); // Small delay for LVGL
}