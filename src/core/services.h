#pragma once
#include <memory>
#include "driver_interface.h"

// Forward declarations
class AudioManager;
class BLEKeyboard;
class WiFiControl;
class DisplayManager;
class KeyProcessor;

/**
 * @brief Centralized service registry for managing all system services
 * 
 * This singleton provides access to all system services and manages
 * their lifecycle. This eliminates the need for global objects and
 * provides a clean interface for dependency injection.
 */
class Services {
public:
    static Services& instance();
    
    // Service accessors
    AudioManager& audio();
    BLEKeyboard& bleKeyboard();
    WiFiControl& wifi();
    DisplayManager& display();
    KeyProcessor& keyProcessor();
    
    // Lifecycle management
    bool initialize();
    void shutdown();
    
    // Status checking
    bool isSystemReady() const;
    
private:
    Services() = default;
    ~Services() = default;
    
    // Disable copy constructor and assignment operator
    Services(const Services&) = delete;
    Services& operator=(const Services&) = delete;
    
    // Service instances
    std::unique_ptr<AudioManager> audioManager_;
    std::unique_ptr<BLEKeyboard> bleKeyboard_;
    std::unique_ptr<WiFiControl> wifiControl_;
    std::unique_ptr<DisplayManager> displayManager_;
    std::unique_ptr<KeyProcessor> keyProcessor_;
    
    // Initialization state
    bool initialized_ = false;
};
