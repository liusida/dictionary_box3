#pragma once
#include "core/service_manager.h"
#include "controllers/main_screen.h"
#include "controllers/splash_screen.h"
#include "controllers/wifi_settings_screen.h"
#include "controllers/keyboard_settings_screen.h"
#include "main.h"

/**
 * @brief Main application coordinator
 * 
 * Manages the overall application state and coordinates between
 * different screen managers and services.
 */
class App {
public:
    App();
    ~App();
    
    /**
     * @brief Initialize the application
     * @return true if successful, false otherwise
     */
    bool initialize();
    
    /**
     * @brief Shutdown the application
     */
    void shutdown();
    
    /**
     * @brief Main application tick - call this in loop()
     */
    void tick();
    
    /**
     * @brief Enter splash state
     */
    void enterSplashState();
    
    /**
     * @brief Enter main state
     */
    void enterMainState();
    void enterWiFiSettingsState();
    void enterKeyboardSettingsState();
    
    /**
     * @brief Check if system is ready
     * @return true if ready, false otherwise
     */
    bool isSystemReady() const;
    
    /**
     * @brief Get current application state
     * @return Current state
     */
    AppState getCurrentState() const;
    
private:
    ServiceManager& serviceManager_;
    SplashScreen splashScreen_;
    MainScreen mainScreen_;
    WiFiSettingsScreen wifiSettingsScreen_;
    KeyboardSettingsScreen keyboardSettingsScreen_;
    AppState currentState_;
    bool stateTransitioned_;
    unsigned long lastStateCheck_;
    bool initialized_;
    
    // State management
    void updateStateMachine();
};
