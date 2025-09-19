#include "app.h"
#include "core/event_system.h"
#include "ui/ui.h"
#include "drivers/lvgl_drive.h"
#include "drivers/display_manager.h"
#include "drivers/ble_keyboard.h"
#include "drivers/wifi_control.h"
#include "core/log.h"

static const char *TAG = "App";

App::App() 
    : services_(Services::instance()), 
      mainScreen_(services_),
      currentState_(STATE_SPLASH),
      stateTransitioned_(false),
      lastStateCheck_(0),
      initialized_(false) {
}

App::~App() {
    shutdown();
}

bool App::initialize() {
    if (initialized_) {
        return true;
    }
    
    ESP_LOGI(TAG, "Initializing application controller...");
    
    if (!mainScreen_.initialize()) {
        ESP_LOGE(TAG, "Failed to initialize main screen");
        return false;
    }
    // Initialize WiFi settings screen
    wifiSettingsScreen_.initialize();
    
    // Initialize keyboard settings screen
    keyboardSettingsScreen_.initialize();
    
    initialized_ = true;
    ESP_LOGI(TAG, "Application controller initialized");
    return true;
}

void App::shutdown() {
    if (!initialized_) {
        return;
    }
    
    ESP_LOGI(TAG, "Shutting down application...");
    mainScreen_.shutdown();
    wifiSettingsScreen_.shutdown();
    keyboardSettingsScreen_.shutdown();
    initialized_ = false;
}

void App::tick() {
    if (!initialized_) {
        return;
    }
    
    // Process all queued events first (this prevents stack overflow from BLE callbacks)
    EventSystem::instance().processAllEvents();
    
    // Handle LVGL tasks
    services_.display().tick();
    
    // Tick all services
    services_.bleKeyboard().tick();
    services_.wifi().tick();
    services_.keyProcessor().tick();
    
    // Update state machine
    updateStateMachine();
}

void App::enterSplashState() {
    ESP_LOGI(TAG, "Entering SPLASH state");
    currentState_ = STATE_SPLASH;
    stateTransitioned_ = true;
    loadScreen(ui_Splash);
}

void App::enterMainState() {
    ESP_LOGI(TAG, "Entering MAIN state");
    currentState_ = STATE_MAIN;
    stateTransitioned_ = true;
    mainScreen_.enterMainState();
}

void App::enterWiFiSettingsState() {
    ESP_LOGI(TAG, "Entering WIFI_SETTINGS state");
    currentState_ = STATE_WIFI_SETTINGS;
    stateTransitioned_ = true;
    wifiSettingsScreen_.enterWiFiSettingsState();
}

void App::enterKeyboardSettingsState() {
    ESP_LOGI(TAG, "Entering KEYBOARD_SETTINGS state");
    currentState_ = STATE_KEYBOARD_SETTINGS;
    stateTransitioned_ = true;
    keyboardSettingsScreen_.enterKeyboardSettingsState();
}

bool App::isSystemReady() const {
    return services_.isSystemReady();
}

AppState App::getCurrentState() const {
    return currentState_;
}

void App::updateStateMachine() {
    switch (currentState_) {
        case STATE_SPLASH:
            // Check if WiFi is not connected first
            if (!services_.wifi().isConnected()) {
                enterWiFiSettingsState();
            }
            // Then check if system is ready to transition to main
            else if (isSystemReady()) {
                enterMainState();
            }
            break;
            
        case STATE_MAIN:
            // Just stay here - main screen controller handles everything
            break;
        case STATE_WIFI_SETTINGS:
            // If WiFi connected, return to main
            if (services_.wifi().isConnected()) {
                enterMainState();
            }
            break;
            
        case STATE_KEYBOARD_SETTINGS:
            // If BLE connected, return to main
            if (services_.bleKeyboard().isConnected()) {
                enterMainState();
            }
            break;
    }
}
