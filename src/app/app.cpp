#include "app.h"
#include "screens/splash_screen.h"
#include "screens/main_screen.h"
#include "screens/wifi_settings_screen.h"
#include "screens/keyboard_settings_screen.h"
#include "drivers/display_manager.h"
#include "audio_manager.h"
#include "network_control.h"
#include "drivers/ble_keyboard.h"
#include "core/log.h"
#include <memory>

static const char *TAG = "App";

App::App() 
    : stateManager_(StateManager::instance()) {
}

App::~App() {
    shutdown();
}

bool App::initialize() {
    if (initialized_) {
        return true;
    }
    
    ESP_LOGI(TAG, "Initializing application controller...");
    
    // Initialize drivers in correct order (following simplified architecture)
    if (!initializeDrivers(true)) {
        ESP_LOGE(TAG, "Failed to initialize drivers");
        return false;
    }
    
    // Initialize state manager
    stateManager_.initialize();
    stateManager_.setAppController(this);
    
    // Initialize screen controllers now that display driver is ready
    initializeScreenControllers();
    
    initialized_ = true;
    ESP_LOGI(TAG, "Application controller initialized");
    return true;
}

bool App::initializeRemainingComponents() {
    ESP_LOGI(TAG, "Initializing remaining components...");
    
    // Initialize remaining drivers (WiFi, Audio, BLE)
    if (!initializeDrivers(false)) {
        ESP_LOGE(TAG, "Failed to initialize remaining drivers");
        return false;
    }
    
    // Do not re-create screen controllers here. They were created after display init.
    // Screens are tolerant of null drivers and will function once drivers are ready.
    
    ESP_LOGI(TAG, "All components initialized successfully");
    return true;
}

void App::shutdown() {
    if (!initialized_) {
        return;
    }
    
    ESP_LOGI(TAG, "Shutting down application...");
    
    // Shutdown screen controllers
    if (mainScreen_) {
        mainScreen_->shutdown();
    }
    if (wifiSettingsScreen_) {
        wifiSettingsScreen_->shutdown();
    }
    if (keyboardSettingsScreen_) {
        keyboardSettingsScreen_->shutdown();
    }
    
    // Shutdown state manager
    stateManager_.shutdown();
    
    // Shutdown drivers (following simplified architecture)
    if (bleDriver_) {
        bleDriver_->shutdown();
    }
    if (audioDriver_) {
        audioDriver_->shutdown();
    }
    if (wifiDriver_) {
        wifiDriver_->shutdown();
    }
    if (displayDriver_) {
        displayDriver_->shutdown();
    }
    
    initialized_ = false;
}

void App::tick() {
    if (!initialized_) {
        return;
    }
    
    // Monitor driver health and handle recovery (following simplified architecture)
    monitorDriverHealth();
    
    // Tick state manager
    stateManager_.tick();
    
    // Tick current screen controller
    AppState currentState = stateManager_.getCurrentState();
    switch (currentState) {
        case AppState::SPLASH:
            if (splashScreen_) {
                splashScreen_->tick();
                
                // Check if splash screen should exit
                if (splashScreen_->shouldExitSplash()) {
                    // Check WiFi status to determine next state (following simplified architecture)
                    if (wifiDriver_->isConnected()) {
                        ESP_LOGI(TAG, "Splash screen ready - WiFi connected, transitioning to main state");
                        enterMainState();
                    } else {
                        ESP_LOGI(TAG, "Splash screen ready - WiFi not connected, transitioning to WiFi settings");
                        enterWiFiSettingsState();
                    }
                }
            }
            break;
        case AppState::MAIN:
            if (mainScreen_) {
                mainScreen_->tick();
            }
            break;
        case AppState::WIFI_SETTINGS:
            if (wifiSettingsScreen_) {
                wifiSettingsScreen_->tick();
            }
            break;
        case AppState::KEYBOARD_SETTINGS:
            if (keyboardSettingsScreen_) {
                keyboardSettingsScreen_->tick();
            }
            break;
    }
    
    // Update state machine
    updateStateMachine();
}

void App::enterSplashState() {
    ESP_LOGI(TAG, "Entering SPLASH state");
    stateManager_.setState(AppState::SPLASH);
    
    if (splashScreen_) {
        splashScreen_->enterSplashState();
        splashScreen_->startConnectivityInitialization();
    } else {
        ESP_LOGI(TAG, "Splash screen controller not initialized yet - UI splash screen should be visible");
        // For display-only initialization, the splash screen is already loaded by ui_init()
        // We just need to set the state and let the UI handle the display
    }
}

void App::enterMainState() {
    ESP_LOGI(TAG, "Entering MAIN state");
    stateManager_.setState(AppState::MAIN);
    
    if (mainScreen_) {
        mainScreen_->showMainScreen();
    }
}

void App::enterWiFiSettingsState() {
    ESP_LOGI(TAG, "Entering WIFI_SETTINGS state");
    stateManager_.setState(AppState::WIFI_SETTINGS);
    
    if (wifiSettingsScreen_) {
        wifiSettingsScreen_->showWiFiSettingsScreen();
    }
}

void App::enterKeyboardSettingsState() {
    ESP_LOGI(TAG, "Entering KEYBOARD_SETTINGS state");
    stateManager_.setState(AppState::KEYBOARD_SETTINGS);
    
    if (keyboardSettingsScreen_) {
        keyboardSettingsScreen_->showKeyboardSettingsScreen();
    }
}

bool App::isSystemReady() const {
    // Check if all essential drivers are ready (following simplified architecture)
    return wifiDriver_ && wifiDriver_->isConnected() && 
           bleDriver_ && bleDriver_->isConnected();
}

AppState App::getCurrentState() const {
    return stateManager_.getCurrentState();
}

void App::updateStateMachine() {
    AppState currentState = stateManager_.getCurrentState();
    
    switch (currentState) {
        case AppState::SPLASH:
            // Check if splash screen is ready to transition
            if (splashScreen_ && splashScreen_->isConnectivityInitialized()) {
                // Connectivity is ready, check what to do next (following simplified architecture)
                bool wifiConnected = wifiDriver_ ? wifiDriver_->isConnected() : false;
                bool bleConnected = bleDriver_ ? bleDriver_->isConnected() : false;
                
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
            
        case AppState::MAIN:
            // Just stay here - main screen controller handles everything
            break;
            
        case AppState::WIFI_SETTINGS:
            // If WiFi connected, return to main (following simplified architecture)
            if (wifiDriver_ && wifiDriver_->isConnected()) {
                enterMainState();
            }
            if (bleDriver_ && !bleDriver_->isConnected()) {
                enterKeyboardSettingsState();
            }
            break;
            
        case AppState::KEYBOARD_SETTINGS:
            // If BLE connected, return to main (following simplified architecture)
            if (bleDriver_ && bleDriver_->isConnected()) {
                enterMainState();
            }
            break;
    }
}

void App::initializeScreenControllers() {
    ESP_LOGI(TAG, "Initializing screen controllers...");
    
    // Create screen controller instances (following simplified architecture)
    splashScreen_ = std::make_unique<SplashScreen>(wifiDriver_.get(), bleDriver_.get());
    mainScreen_ = std::make_unique<MainScreen>(*wifiDriver_, *audioDriver_, *bleDriver_);
    wifiSettingsScreen_ = std::make_unique<WiFiSettingsScreen>(*wifiDriver_);
    keyboardSettingsScreen_ = std::make_unique<KeyboardSettingsScreen>(*bleDriver_);
    
    // Initialize screen controllers
    if (!splashScreen_->initialize()) {
        ESP_LOGE(TAG, "Failed to initialize splash screen");
        return;
    }
    
    if (!mainScreen_->initialize()) {
        ESP_LOGE(TAG, "Failed to initialize main screen");
        return;
    }
    
    if (!wifiSettingsScreen_->initialize()) {
        ESP_LOGE(TAG, "Failed to initialize WiFi settings screen");
        return;
    }
    
    if (!keyboardSettingsScreen_->initialize()) {
        ESP_LOGE(TAG, "Failed to initialize keyboard settings screen");
        return;
    }
    
    ESP_LOGI(TAG, "All screen controllers initialized");
}


bool App::initializeDrivers(bool onlyInitializeDisplay) {
    ESP_LOGI(TAG, "Initializing drivers in correct order...");
    
    // Initialize in dependency order (following simplified architecture)
    // 1. Display driver - needed for UI
    if (!displayDriver_) {
        displayDriver_ = std::make_unique<DisplayManager>();
        if (!displayDriver_->initialize()) {
            ESP_LOGE(TAG, "Failed to initialize display driver");
            return false;
        }
    }
    if (onlyInitializeDisplay) {
        ESP_LOGI(TAG, "Display driver initialized successfully");
        return true;
    }
    
    // 2. WiFi driver - needed for audio streaming
    wifiDriver_ = std::make_unique<NetworkControl>();
    if (!wifiDriver_->initialize()) {
        ESP_LOGE(TAG, "Failed to initialize WiFi driver");
        return false;
    }
    
    // 3. Audio driver - depends on WiFi
    audioDriver_ = std::make_unique<AudioManager>();
    if (!audioDriver_->initialize()) {
        ESP_LOGE(TAG, "Failed to initialize audio driver");
        return false;
    }
    
    // 4. BLE driver - independent
    bleDriver_ = std::make_unique<BLEKeyboard>();
    if (!bleDriver_->initialize()) {
        ESP_LOGE(TAG, "Failed to initialize BLE driver");
        return false;
    }
    
    // Note: Input handling is done directly through BLE driver callbacks
    
    ESP_LOGI(TAG, "All drivers initialized successfully");
    return true;
}


void App::monitorDriverHealth() {
    // Monitor driver health and handle recovery (following simplified architecture)
    AppState currentState = stateManager_.getCurrentState();
    
    // Check WiFi health (only if WiFi driver is initialized)
    if (wifiDriver_ && !wifiDriver_->isConnected() && currentState == AppState::MAIN) {
        ESP_LOGW(TAG, "WiFi connection lost, transitioning to WiFi settings");
        enterWiFiSettingsState();
    }
    
    // Check BLE health (only if BLE driver is initialized)
    if (bleDriver_ && !bleDriver_->isConnected() && currentState == AppState::MAIN) {
        ESP_LOGW(TAG, "BLE connection lost, transitioning to keyboard settings");
        enterKeyboardSettingsState();
    }
    
    // Check for recovery (only if drivers are initialized)
    if (currentState == AppState::WIFI_SETTINGS && wifiDriver_ && wifiDriver_->isConnected()) {
        ESP_LOGI(TAG, "WiFi reconnected, returning to main");
        enterMainState();
    }
    
    if (currentState == AppState::KEYBOARD_SETTINGS && bleDriver_ && bleDriver_->isConnected()) {
        ESP_LOGI(TAG, "BLE reconnected, returning to main");
        enterMainState();
    }
}
