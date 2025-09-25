#pragma once
#include <Arduino.h>

// Forward declarations
class App;

/**
 * @brief Application states
 */
enum class AppState { 
    SPLASH, 
    MAIN, 
    WIFI_SETTINGS, 
    KEYBOARD_SETTINGS 
};

/**
 * @brief Unified state management for the application
 * 
 * Replaces the complex App state machine + RuntimeStateManager + ConnectionMonitor
 * architecture with a single, unified state manager.
 */
class StateManager {
public:
    static StateManager& instance();
    
    // Lifecycle
    void initialize();
    void shutdown();
    void tick(); // Call in main loop
    
    // State management
    void setAppController(App* app);
    AppState getCurrentState() const;
    void setState(AppState newState);
    
    // Driver health monitoring (following simplified architecture)
    void checkDriverHealth();
    void handleDriverFailure(const String& driver);
    void handleDriverRecovery(const String& driver);
    
    // Manual state transitions
    void requestWiFiSettings();
    void requestKeyboardSettings();
    void returnToMain();
    
    // Status
    bool isInRecoveryMode() const;
    String getCurrentRecoveryService() const;
    
private:
    StateManager() = default;
    ~StateManager() = default;
    
    // Disable copy constructor and assignment operator
    StateManager(const StateManager&) = delete;
    StateManager& operator=(const StateManager&) = delete;
    
    // State tracking
    App* appController_ = nullptr;
    AppState currentState_ = AppState::SPLASH;
    bool stateTransitioned_ = false;
    
    // Service health tracking
    bool wifiHealthy_ = false;
    bool bleHealthy_ = false;
    bool wifiFailed_ = false;
    bool bleFailed_ = false;
    bool wifiRecovered_ = false;
    bool bleRecovered_ = false;
    
    // Recovery mode
    bool inRecoveryMode_ = false;
    String currentRecoveryService_;
    unsigned long lastRecoveryTime_ = 0;
    unsigned long recoveryCooldown_ = 5000; // 5 seconds between recovery attempts
    
    // Health check timing
    unsigned long lastHealthCheck_ = 0;
    static const unsigned long HEALTH_CHECK_INTERVAL_MS = 2000; // Check every 2 seconds
    
    // Internal methods
    void transitionToRecoveryScreen(const String& service);
    void checkForRecovery();
    bool shouldShowRecoveryScreen(const String& service) const;
    void logRecoveryStatus();
};
