#pragma once
#include "state_manager.h"
#include <memory>

// Driver forward declarations
class DisplayManager;
class AudioManager;
class WiFiControl;
class BLEKeyboard;

// Screen controller forward declarations
class SplashScreen;
class MainScreen;
class WiFiSettingsScreen;
class KeyboardSettingsScreen;

/**
 * @brief Simplified main application controller
 * 
 * Coordinates between drivers, state management, and screen controllers.
 * Follows simplified architecture: App Controller + Screen Controllers + Drivers.
 */
class App {
public:
    // Construction
    App();
    ~App();
    
    // Lifecycle
    bool initialize();
    bool initializeRemainingComponents();
    void shutdown();
    void tick();
    
    // State transitions
    void enterSplashState();
    void enterMainState();
    void enterWiFiSettingsState();
    void enterKeyboardSettingsState();
    
    // Queries
    AppState getCurrentState() const;
    bool isSystemReady() const;
    
private:
    // Helpers
    void updateStateMachine();
    void initializeScreenControllers();
    bool initializeDrivers(bool onlyInitializeDisplay = false);
    void monitorDriverHealth();
    
    // State manager
    StateManager& stateManager_;
    
    // Initialization state
    bool initialized_ = false;
    
    // Drivers (following simplified architecture)
    std::unique_ptr<DisplayManager> displayDriver_;
    std::unique_ptr<AudioManager> audioDriver_;
    std::unique_ptr<WiFiControl> wifiDriver_;
    std::unique_ptr<BLEKeyboard> bleDriver_;
    
    // Screen controllers
    std::unique_ptr<SplashScreen> splashScreen_;
    std::unique_ptr<MainScreen> mainScreen_;
    std::unique_ptr<WiFiSettingsScreen> wifiSettingsScreen_;
    std::unique_ptr<KeyboardSettingsScreen> keyboardSettingsScreen_;
};
