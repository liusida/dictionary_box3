#include "app_controller.h"
#include "core/event_system.h"
#include "ui/ui.h"
#include "drivers/lvgl_drive.h"
#include "drivers/display_manager.h"
#include "drivers/ble_keyboard.h"
#include "drivers/wifi_control.h"
#include "esp_log.h"

static const char *TAG = "AppController";

AppController::AppController() 
    : services_(Services::instance()), 
      mainScreenController_(services_),
      currentState_(STATE_SPLASH),
      stateTransitioned_(false),
      lastStateCheck_(0),
      initialized_(false) {
}

AppController::~AppController() {
    shutdown();
}

bool AppController::initialize() {
    if (initialized_) {
        return true;
    }
    
    ESP_LOGI(TAG, "Initializing application controller...");
    
    if (!mainScreenController_.initialize()) {
        ESP_LOGE(TAG, "Failed to initialize main screen controller");
        return false;
    }
    
    initialized_ = true;
    ESP_LOGI(TAG, "Application controller initialized");
    return true;
}

void AppController::shutdown() {
    if (!initialized_) {
        return;
    }
    
    ESP_LOGI(TAG, "Shutting down application controller...");
    mainScreenController_.shutdown();
    initialized_ = false;
}

void AppController::tick() {
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

void AppController::enterSplashState() {
    ESP_LOGI(TAG, "Entering SPLASH state");
    currentState_ = STATE_SPLASH;
    stateTransitioned_ = true;
    loadScreen(ui_Splash);
}

void AppController::enterMainState() {
    ESP_LOGI(TAG, "Entering MAIN state");
    currentState_ = STATE_MAIN;
    stateTransitioned_ = true;
    mainScreenController_.enterMainState();
}

bool AppController::isSystemReady() const {
    return services_.isSystemReady();
}

AppState AppController::getCurrentState() const {
    return currentState_;
}

void AppController::updateStateMachine() {
    switch (currentState_) {
        case STATE_SPLASH:
            // Check if system is ready to transition to main
            if (isSystemReady()) {
                enterMainState();
            }
            break;
            
        case STATE_MAIN:
            // Just stay here - main screen controller handles everything
            break;
    }
}
