#pragma once
#include "core/services.h"
#include "controllers/main_screen_controller.h"
#include "main.h"

/**
 * @brief Main application controller
 * 
 * Manages the overall application state and coordinates between
 * different controllers and services.
 */
class AppController {
public:
    AppController();
    ~AppController();
    
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
    Services& services_;
    MainScreenController mainScreenController_;
    AppState currentState_;
    bool stateTransitioned_;
    unsigned long lastStateCheck_;
    bool initialized_;
    
    // State management
    void updateStateMachine();
};
