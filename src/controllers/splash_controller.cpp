#include "splash_controller.h"
#include "drivers/lvgl_drive.h"
#include "esp_log.h"

static const char* TAG = "SplashController";

SplashController::SplashController() : initialized_(false), splashStartTime_(0) {}

SplashController::~SplashController() {
    shutdown();
}

bool SplashController::initialize() {
    ESP_LOGI(TAG, "Initializing splash controller...");
    initialized_ = true;
    return true;
}

void SplashController::shutdown() {
    ESP_LOGI(TAG, "Shutting down splash controller...");
    initialized_ = false;
}

void SplashController::tick() {
    if (!initialized_) return;
    
    // Check if splash duration has elapsed
    if (splashStartTime_ > 0 && millis() - splashStartTime_ >= SPLASH_DURATION_MS) {
        exitSplashState();
    }
}

void SplashController::enterSplashState() {
    ESP_LOGI(TAG, "Entering splash state...");
    loadScreen(ui_Splash);
    splashStartTime_ = millis();
}

void SplashController::exitSplashState() {
    ESP_LOGI(TAG, "Exiting splash state...");
    splashStartTime_ = 0;
    // Transition to main screen will be handled by AppController
}
