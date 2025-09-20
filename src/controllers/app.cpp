#include "app.h"
#include "core/event_system.h"
#include "core/connection_monitor.h"
#include "core/runtime_state_manager.h"
#include "drivers/display_manager.h"
#include "drivers/wifi_control.h"
#include "drivers/ble_keyboard.h"
#include "ui/ui.h"
#include "drivers/lvgl_drive.h"
#include "core/log.h"

static const char *TAG = "App";

App::App() 
    : serviceManager_(ServiceManager::instance()), 
      splashScreen_(),
      mainScreen_(serviceManager_),
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
    
    if (!splashScreen_.initialize()) {
        ESP_LOGE(TAG, "Failed to initialize splash screen");
        return false;
    }
    
    if (!mainScreen_.initialize()) {
        ESP_LOGE(TAG, "Failed to initialize main screen");
        return false;
    }
    // Initialize WiFi settings screen
    wifiSettingsScreen_.initialize();
    
    // Initialize keyboard settings screen
    keyboardSettingsScreen_.initialize();
    
    // Initialize runtime state management
    ConnectionMonitor::instance().initialize();
    RuntimeStateManager::instance().initialize();
    RuntimeStateManager::instance().setAppController(this);
    
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
    serviceManager_.display().tick();
    
    // Tick all services
    serviceManager_.bleKeyboard().tick();
    serviceManager_.wifi().tick();
    serviceManager_.keyProcessor().tick();
    
    // Tick splash screen (handles connectivity initialization)
    splashScreen_.tick();
    
    // Monitor connection health and handle runtime failures
    ConnectionMonitor::instance().tick();
    RuntimeStateManager::instance().tick();
    
    // Update state machine
    updateStateMachine();
}

void App::enterSplashState() {
    ESP_LOGI(TAG, "Entering SPLASH state");
    currentState_ = STATE_SPLASH;
    stateTransitioned_ = true;
    
    // Start splash screen and connectivity initialization
    splashScreen_.enterSplashState();
    splashScreen_.startConnectivityInitialization();
    
    // Force LVGL refresh to ensure splash screen is visible
    serviceManager_.display().tick();
    
    // Give the display time to render
    delay(100);
    
    ESP_LOGI(TAG, "Splash screen loaded and connectivity initialization started");
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
    return serviceManager_.isSystemReady();
}

AppState App::getCurrentState() const {
    return currentState_;
}

void App::updateStateMachine() {
    switch (currentState_) {
        case STATE_SPLASH:
            // Check if splash screen is ready to transition
            if (splashScreen_.isConnectivityInitialized()) {
                // Connectivity is ready, check what to do next
                bool wifiConnected = serviceManager_.wifi().isConnected();
                bool bleConnected = serviceManager_.bleKeyboard().isConnected();
                
                ESP_LOGI(TAG, "Splash transition check - WiFi: %s, BLE: %s", 
                         wifiConnected ? "Connected" : "Disconnected",
                         bleConnected ? "Connected" : "Disconnected");
                
                if (!wifiConnected) {
                    ESP_LOGI(TAG, "Transitioning to WiFi settings");
                    enterWiFiSettingsState();
                } else if (!bleConnected) {
                    // WiFi is connected but BLE keyboard is not - go to keyboard settings
                    ESP_LOGI(TAG, "Transitioning to keyboard settings");
                    enterKeyboardSettingsState();
                } else if (isSystemReady()) {
                    ESP_LOGI(TAG, "Transitioning to main screen");
                    enterMainState();
                }
            }
            break;
            
        case STATE_MAIN:
            // Just stay here - main screen controller handles everything
            break;
        case STATE_WIFI_SETTINGS:
            // If WiFi connected, return to main
            if (serviceManager_.wifi().isConnected()) {
                enterMainState();
            }
            if (!serviceManager_.bleKeyboard().isConnected()) {
                enterKeyboardSettingsState();
            }
            break;
            
        case STATE_KEYBOARD_SETTINGS:
            // If BLE connected, return to main
            if (serviceManager_.bleKeyboard().isConnected()) {
                enterMainState();
            }
            break;
    }
}
