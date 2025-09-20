#include "splash_screen.h"
#include "drivers/lvgl_drive.h"
#include "core/service_manager.h"
#include "drivers/ble_keyboard.h"
#include "drivers/wifi_control.h"
#include "input/key_processor.h"
#include "core/log.h"

static const char* TAG = "SplashScreen";

SplashScreen::SplashScreen() : initialized_(false), splashStartTime_(0), 
                              connectivityStarted_(false), connectivityInitialized_(false) {}

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
    
    // Update splash progress
    updateSplashProgress();
    
    // Check connectivity status
    checkConnectivityStatus();
    
    // Check if we should exit splash screen
    if (splashStartTime_ > 0) {
        unsigned long elapsed = millis() - splashStartTime_;
        
        // Exit if minimum splash time has elapsed AND connectivity is ready OR timeout reached
        if ((elapsed >= SPLASH_DURATION_MS && connectivityInitialized_) || 
            elapsed >= CONNECTIVITY_TIMEOUT_MS) {
            exitSplashState();
        }
    }
}

void SplashScreen::enterSplashState() {
    ESP_LOGI(TAG, "Entering splash state...");
    loadScreen(ui_Splash);
    lv_label_set_text(ui_LoadingTxt, "Please press a key to wake the keyboard...");
    splashStartTime_ = millis();
    
    // Force display refresh to ensure splash screen is visible
    lv_refr_now(NULL);
    
    // Initialize loading bar and text
    if (ui_LoadingBar) {
        lv_bar_set_value(ui_LoadingBar, 0, LV_ANIM_OFF);
        lv_bar_set_range(ui_LoadingBar, 0, 100);
    }
    
    if (ui_LoadingTxt) {
        lv_label_set_text(ui_LoadingTxt, "Initializing...");
    }
    
    ESP_LOGI(TAG, "Splash screen loaded and should be visible");
}

void SplashScreen::exitSplashState() {
    ESP_LOGI(TAG, "Exiting splash state...");
    splashStartTime_ = 0;
    // Transition to main screen will be handled by AppController
}

void SplashScreen::startConnectivityInitialization() {
    if (connectivityStarted_) {
        return;
    }
    
    ESP_LOGI(TAG, "Starting connectivity services initialization...");
    ServiceManager::instance().startConnectivityInitialization();
    connectivityStarted_ = true;
    
    // Set up BLE keyboard callback
    ServiceManager::instance().bleKeyboard().setKeyCallback([](char key, uint8_t keyCode, uint8_t modifiers) {
        ServiceManager::instance().keyProcessor().sendKeyToLVGL(key, keyCode, modifiers);
    });
}

bool SplashScreen::isConnectivityInitialized() const {
    return connectivityInitialized_;
}

void SplashScreen::updateSplashProgress() {
    if (splashStartTime_ == 0) return;
    
    unsigned long elapsed = millis() - splashStartTime_;
    unsigned long progress = min(100UL, (elapsed * 100) / CONNECTIVITY_TIMEOUT_MS);
    
    // Update loading bar if it exists
    if (ui_LoadingBar) {
        lv_bar_set_value(ui_LoadingBar, progress, LV_ANIM_ON);
        ESP_LOGD(TAG, "Updated loading bar to %lu%%", progress);
    } else {
        ESP_LOGW(TAG, "ui_LoadingBar is NULL!");
    }
    
    // Update loading text
    if (ui_LoadingTxt) {
        const char* text = "Initializing...";
        if (connectivityInitialized_) {
            // Check what's ready
            bool wifiReady = ServiceManager::instance().wifi().isConnected();
            bool bleReady = ServiceManager::instance().bleKeyboard().isConnected();
            
            if (wifiReady && bleReady) {
                text = "Ready!";
            } else if (wifiReady && !bleReady) {
                text = "WiFi Ready - Setting up keyboard...";
            } else {
                text = "Connecting...";
            }
        } else if (connectivityStarted_) {
            text = "Connecting...";
        }
        lv_label_set_text(ui_LoadingTxt, text);
        ESP_LOGD(TAG, "Updated loading text to: %s", text);
    } else {
        ESP_LOGW(TAG, "ui_LoadingTxt is NULL!");
    }
}

void SplashScreen::checkConnectivityStatus() {
    if (!connectivityStarted_) {
        return;
    }
    
    // Check if connectivity services are ready
    bool wifiReady = ServiceManager::instance().wifi().isConnected();
    bool bleReady = ServiceManager::instance().bleKeyboard().isConnected();
    
    ESP_LOGD(TAG, "Connectivity status - WiFi: %s, BLE: %s", 
             wifiReady ? "Connected" : "Disconnected",
             bleReady ? "Connected" : "Disconnected");
    
    // Check if we should mark connectivity as initialized
    if (!connectivityInitialized_) {
        // If WiFi is ready, we can proceed (BLE is optional)
        if (wifiReady) {
            connectivityInitialized_ = true;
            ESP_LOGI(TAG, "Connectivity services ready - WiFi: %s, BLE: %s", 
                     wifiReady ? "Connected" : "Disconnected",
                     bleReady ? "Connected" : "Disconnected");
        }
    }
}
