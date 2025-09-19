#include "splash_screen.h"
#include "drivers/lvgl_drive.h"
#include "core/log.h"

static const char* TAG = "SplashScreen";

SplashScreen::SplashScreen() : initialized_(false), splashStartTime_(0) {}

SplashScreen::~SplashScreen() {
    shutdown();
}

bool SplashScreen::initialize() {
    ESP_LOGI(TAG, "Initializing splash controller...");
    initialized_ = true;
    return true;
}

void SplashScreen::shutdown() {
    ESP_LOGI(TAG, "Shutting down splash controller...");
    initialized_ = false;
}

void SplashScreen::tick() {
    if (!initialized_) return;
    
    // Check if splash duration has elapsed
    if (splashStartTime_ > 0 && millis() - splashStartTime_ >= SPLASH_DURATION_MS) {
        exitSplashState();
    }
}

void SplashScreen::enterSplashState() {
    ESP_LOGI(TAG, "Entering splash state...");
    loadScreen(ui_Splash);
    splashStartTime_ = millis();
}

void SplashScreen::exitSplashState() {
    ESP_LOGI(TAG, "Exiting splash state...");
    splashStartTime_ = 0;
    // Transition to main screen will be handled by AppController
}
