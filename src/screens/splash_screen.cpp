#include "splash_screen.h"
#include "drivers/lvgl_drive.h"
#include "network_control.h"
#include "drivers/ble_keyboard.h"
#include "core/log.h"

static const char* TAG = "SplashScreen";

SplashScreen::SplashScreen(NetworkControl* wifiDriver, BLEKeyboard* bleDriver) 
    : wifiDriver_(wifiDriver), bleDriver_(bleDriver),
      initialized_(false), 
      splashStartTime_(0), 
      connectivityStarted_(false), 
      connectivityInitialized_(false), 
      wifiConnectionFailed_(false),
      lastWifiStatus_(false), 
      lastBleStatus_(false) {
}

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
    
    // Update status icons based on driver state (following simplified architecture)
    // updateStatusIcons();
    
    // Exit logic is now handled by App controller via shouldExitSplash()
}

void SplashScreen::updateStatusIcons() {
    // Update WiFi status icon based on driver state (following simplified architecture)
    if (wifiDriver_ && wifiDriver_->isConnected()) {
        // TODO: Set WiFi icon to green/connected state
        ESP_LOGD(TAG, "Splash: WiFi connected - updating status icon");
    } else {
        // TODO: Set WiFi icon to red/disconnected state
        ESP_LOGD(TAG, "Splash: WiFi disconnected - updating status icon");
    }
    
    // Update BLE status icon based on driver state
    if (bleDriver_ && bleDriver_->isConnected()) {
        // TODO: Set BLE icon to blue/connected state
        ESP_LOGD(TAG, "Splash: BLE connected - updating status icon");
    } else {
        // TODO: Set BLE icon to red/disconnected state
        ESP_LOGD(TAG, "Splash: BLE disconnected - updating status icon");
    }
}

void SplashScreen::enterSplashState() {
    ESP_LOGI(TAG, "Entering splash state...");
    // Screen is already initialized by ui_init(), just show it
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

bool SplashScreen::shouldExitSplash() const {
    if (splashStartTime_ == 0) {
        return false;
    }
    
    unsigned long elapsed = millis() - splashStartTime_;
    
    // Exit if minimum splash time has elapsed AND connectivity is ready OR timeout reached
    return (elapsed >= SPLASH_DURATION_MS && connectivityInitialized_) || 
           elapsed >= CONNECTIVITY_TIMEOUT_MS;
}

void SplashScreen::startConnectivityInitialization() {
    if (connectivityStarted_) {
        return;
    }
    
    ESP_LOGI(TAG, "Starting connectivity services initialization...");
    
    // Set up BLE keyboard callback (following simplified architecture)
    if (bleDriver_) {
        bleDriver_->setKeyCallback([](char key, uint8_t keyCode, uint8_t modifiers) {
            // TODO: Handle key input directly or through input driver
            ESP_LOGD(TAG, "BLE key received: %c", key);
        });
    }
    
    connectivityStarted_ = true;
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
    } else {
        ESP_LOGW(TAG, "ui_LoadingBar is NULL!");
    }
    
    // Update loading text
    if (ui_LoadingTxt) {
        const char* text = "Initializing...";
        if (connectivityInitialized_) {
            // Check what's ready (following simplified architecture)
            bool wifiReady = wifiDriver_ ? wifiDriver_->isConnected() : false;
            bool bleReady = bleDriver_ ? bleDriver_->isConnected() : false;
            
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
    } else {
        ESP_LOGW(TAG, "ui_LoadingTxt is NULL!");
    }
}

void SplashScreen::checkConnectivityStatus() {
    if (!connectivityStarted_) {
        return;
    }
    
    // Check if connectivity drivers are ready (following simplified architecture)
    bool wifiReady = wifiDriver_ ? wifiDriver_->isConnected() : false;
    bool bleReady = bleDriver_ ? bleDriver_->isConnected() : false;
    
    // Only log when connectivity status changes
    if (wifiReady != lastWifiStatus_ || bleReady != lastBleStatus_) {
        ESP_LOGD(TAG, "Connectivity status - WiFi: %s, BLE: %s", 
                 wifiReady ? "Connected" : "Disconnected",
                 bleReady ? "Connected" : "Disconnected");
        lastWifiStatus_ = wifiReady;
        lastBleStatus_ = bleReady;
    }
    
    // Check if we should mark connectivity as initialized
    if (!connectivityInitialized_) {
        // If WiFi is ready, we can proceed (BLE is optional)
        if (wifiReady) {
            connectivityInitialized_ = true;
            ESP_LOGI(TAG, "Connectivity services ready - WiFi: %s, BLE: %s", 
                     wifiReady ? "Connected" : "Disconnected",
                     bleReady ? "Connected" : "Disconnected");
        }
        // If WiFi connection has failed, mark as ready to go to WiFi settings
        else if (splashStartTime_ > 0) {
            unsigned long elapsed = millis() - splashStartTime_;
            // Check if WiFi service has attempted connection and failed
            bool wifiConnectionAttempted = elapsed >= 2000; // Give WiFi 2 seconds to attempt connection
            bool wifiConnectionFailed = !wifiReady && wifiConnectionAttempted;
            
            if (wifiConnectionFailed) {
                connectivityInitialized_ = true;
                ESP_LOGI(TAG, "WiFi connection failed - will transition to WiFi settings");
            }
        }
    }
}
