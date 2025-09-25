# Simplified Architecture Design

## Overview

This document outlines the simplified architecture for Dictionary v2, designed to reduce complexity while preserving all functionality. The architecture follows a **two-layer design**: **App Controller** + **Screen Controllers** directly accessing **Drivers**.

## Architecture Principles

1. **Single Responsibility**: Each class has one clear purpose
2. **Minimal Singletons**: Only essential singletons (App, StateManager)
3. **Direct Communication**: Screen controllers directly access drivers
4. **Unified State Management**: Single state manager for all application states
5. **Simplified Initialization**: Linear initialization without complex dependencies

## Directory Structure

```
src/
├── main.cpp                    # Entry point
├── app/                       # Main application
│   ├── app.h/.cpp            # Main app controller
│   └── state_manager.h/.cpp   # Unified state management
├── screens/                   # Screen controllers
│   ├── splash_screen.h/.cpp   # Splash screen UI logic
│   ├── main_screen.h/.cpp     # Main screen UI logic
│   ├── wifi_settings_screen.h/.cpp  # WiFi settings UI logic
│   └── keyboard_settings_screen.h/.cpp  # BLE keyboard settings UI logic
├── drivers/                   # Hardware drivers
│   ├── display_manager.h/.cpp # Display and LVGL management
│   ├── audio_manager.h/.cpp   # Audio playback
│   ├── wifi_control.h/.cpp    # WiFi connection
│   ├── ble_keyboard.h/.cpp    # BLE keyboard
│   └── input_manager.h/.cpp   # Input processing
├── business/                  # Business logic
│   └── dictionary_api.h/.cpp   # Dictionary API communication
├── ui/                       # SquareLine Studio generated UI
│   ├── ui.h/.cpp             # UI initialization
│   ├── ui_*.h/.cpp           # Screen-specific UI files
│   └── ui_assets/            # UI assets
└── core/                     # Core utilities
    └── log.h/.cpp            # Logging utilities
```

## Key Components

### 1. App Controller (`app/app.h/.cpp`)
- **Purpose**: Main application coordinator
- **Responsibilities**: 
  - Initialize all drivers in correct order
  - Manage application state transitions
  - Coordinate between screens and drivers
  - Monitor driver health and handle recovery
- **Simplifications**: 
  - Single initialization sequence
  - Direct method calls
  - Simplified state machine

### 2. State Manager (`app/state_manager.h/.cpp`)
- **Purpose**: Unified state management
- **Responsibilities**:
  - Track application state (SPLASH, MAIN, WIFI_SETTINGS, KEYBOARD_SETTINGS)
  - Handle state transitions
  - Monitor driver health and trigger recovery
- **Replaces**: App state machine + RuntimeStateManager + ConnectionMonitor

### 3. Screen Controllers (`screens/*_screen.h/.cpp`)
- **Purpose**: Handle UI logic for each screen
- **Responsibilities**:
  - Manage screen-specific UI state
  - Handle user interactions
  - Directly access drivers for hardware operations
  - Update status icons based on driver state
- **Simplifications**: Direct driver access, simplified event handling

### 4. Drivers (`drivers/*.h/.cpp`)
- **Display Driver**: Display and LVGL management
- **Audio Driver**: Audio playback and management
- **WiFi Driver**: WiFi connection management
- **BLE Driver**: BLE keyboard management
- **Input Driver**: Keyboard input processing

### 5. Business Logic (`business/dictionary_api.h/.cpp`)
- **Purpose**: Dictionary API communication
- **Responsibilities**: Word lookup, audio streaming
- **Unchanged**: Core business logic preserved

## Data Flow

```
main.cpp
    ↓
App Controller
    ↓
State Manager (tracks current state)
    ↓
Screen Controller (handles UI logic)
    ↓
Drivers (hardware operations)
```

## State Transitions

```
SPLASH → MAIN (if WiFi connected)
SPLASH → WIFI_SETTINGS (if WiFi failed)
SPLASH → KEYBOARD_SETTINGS (if BLE failed)
MAIN → WIFI_SETTINGS (if WiFi lost)
MAIN → KEYBOARD_SETTINGS (if BLE lost)
WIFI_SETTINGS → MAIN (if WiFi connected)
KEYBOARD_SETTINGS → MAIN (if BLE connected)
```

## Functionality Handling

### 1. Dictionary Lookup
- **Screen Controller**: `MainScreen::onWordSubmitted()`
- **Business Logic**: `DictionaryApi::lookupWord()`
- **Driver Access**: `wifiDriver_.isConnected()` for network check
- **UI Update**: Display results in UI elements

### 2. Audio Playback
- **Screen Controller**: `MainScreen::onPlayAudio()`
- **Business Logic**: `DictionaryApi::playAudio()`
- **Driver Access**: `audioDriver_.play()` for audio output
- **Status Icons**: Update audio status icon

### 3. WiFi Management
- **Screen Controller**: `WiFiSettingsScreen::onConnectButtonPressed()`
- **Driver Access**: `wifiDriver_.connect(ssid, password)`
- **Health Monitoring**: `App::tick()` checks `wifiDriver_.isConnected()`
- **Status Icons**: Update WiFi status icon
- **State Transitions**: StateManager transitions to WiFi settings if connection lost

### 4. BLE Keyboard Management
- **Screen Controller**: `KeyboardSettingsScreen::onScanButtonPressed()`
- **Driver Access**: `bleDriver_.startScan()`, `bleDriver_.connectToDevice()`
- **Health Monitoring**: `App::tick()` checks `bleDriver_.isConnected()`
- **Status Icons**: Update BLE status icon
- **State Transitions**: StateManager transitions to keyboard settings if connection lost

### 5. Status Icons Management
- **Problem**: Without services, who updates status icons?
- **Solution**: Each screen controller updates status icons based on driver state
- **Implementation**: 
  ```cpp
  void MainScreen::updateStatusIcons() {
      if (wifiDriver_.isConnected()) {
          lv_obj_set_style_bg_color(ui_WiFiIcon, lv_color_hex(0x00FF00), 0);
      } else {
          lv_obj_set_style_bg_color(ui_WiFiIcon, lv_color_hex(0xFF0000), 0);
      }
  }
  ```

### 6. Health Monitoring & Recovery
- **Problem**: Without services, who monitors driver health?
- **Solution**: App controller monitors drivers and triggers state transitions
- **Implementation**:
  ```cpp
  void App::tick() {
      // Monitor driver health
      if (!wifiDriver_.isConnected() && currentState_ == AppState::MAIN) {
          enterWiFiSettingsState();
      }
      if (!bleDriver_.isConnected() && currentState_ == AppState::MAIN) {
          enterKeyboardSettingsState();
      }
  }
  ```

### 7. Initialization Order
- **Problem**: Some drivers depend on others
- **Solution**: App controller handles initialization sequence
- **Implementation**:
  ```cpp
  bool App::initialize() {
      // Initialize in dependency order
      displayDriver_.initialize();    // First - needed for UI
      wifiDriver_.initialize();       // Second - needed for audio streaming
      audioDriver_.initialize();      // Third - depends on WiFi
      bleDriver_.initialize();        // Fourth - independent
      inputDriver_.initialize();      // Fifth - depends on BLE
      return true;
  }
  ```

## Benefits of Simplified Architecture

### Reduced Complexity
- **60% fewer classes**: From ~25 core classes to ~10 core classes
- **Simpler dependencies**: Clear, linear dependency chain
- **Easier debugging**: Direct method calls instead of complex event flows

### Better Maintainability
- **Single responsibility**: Each class has one clear purpose
- **Clear interfaces**: Direct driver access
- **Linear initialization**: Simple startup sequence

### Preserved Functionality
- **All features maintained**: Dictionary lookup, audio playback, WiFi/BLE management
- **Screen transitions**: Simplified but complete screen management
- **Error handling**: Unified error handling and recovery
- **Status icons**: Manual updates by screen controllers

## Coding Guidelines

### For New Coders

#### 1. **Screen Controllers** - Handle UI Logic + Status Updates
```cpp
// ✅ DO: Handle UI interactions
void MainScreen::onSearchButtonPressed() {
    String word = lv_textarea_get_text(ui_InputWord);
    dictionaryApi_.lookupWord(word);
}

// ✅ DO: Access drivers directly
void WiFiSettingsScreen::onConnectButtonPressed() {
    String ssid = lv_textarea_get_text(ui_InputSSID);
    String password = lv_textarea_get_text(ui_InputPassword);
    wifiDriver_.connect(ssid, password);
}

// ✅ DO: Update status icons manually
void MainScreen::updateStatusIcons() {
    if (wifiDriver_.isConnected()) {
        lv_obj_set_style_bg_color(ui_WiFiIcon, lv_color_hex(0x00FF00), 0);
    } else {
        lv_obj_set_style_bg_color(ui_WiFiIcon, lv_color_hex(0xFF0000), 0);
    }
}

// ❌ DON'T: Handle hardware initialization
// ❌ DON'T: Manage driver lifecycle
// ❌ DON'T: Handle complex error recovery
```

#### 2. **App Controller** - Coordinate Everything + Monitor Health
```cpp
// ✅ DO: Initialize drivers in correct order
bool App::initialize() {
    displayDriver_.initialize();
    wifiDriver_.initialize();
    audioDriver_.initialize();
    bleDriver_.initialize();
    inputDriver_.initialize();
    return true;
}

// ✅ DO: Handle state transitions
void App::enterMainState() {
    stateManager_.setState(AppState::MAIN);
    mainScreen_.showMainScreen();
}

// ✅ DO: Monitor driver health
void App::tick() {
    if (!wifiDriver_.isConnected() && currentState_ == AppState::MAIN) {
        enterWiFiSettingsState();
    }
}

// ❌ DON'T: Handle UI logic
// ❌ DON'T: Access drivers directly from App
```

#### 3. **Drivers** - Handle Hardware Only
```cpp
// ✅ DO: Provide hardware operations
class WiFiDriver {
public:
    bool connect(const String& ssid, const String& password);
    bool isConnected() const;
    IPAddress getIP() const;
};

// ❌ DON'T: Handle UI logic
// ❌ DON'T: Manage application state
```

#### 4. **State Manager** - Track State Only
```cpp
// ✅ DO: Track current state
void StateManager::setState(AppState newState) {
    currentState_ = newState;
}

// ✅ DO: Handle state transitions
void StateManager::checkForRecovery() {
    if (wifiDriver_.isConnected() && currentState_ == AppState::WIFI_SETTINGS) {
        returnToMain();
    }
}

// ❌ DON'T: Handle UI logic
// ❌ DON'T: Initialize drivers
```

### Common Patterns

#### Adding a New Screen
1. Create `screens/new_screen.h/.cpp`
2. Add state to `AppState` enum in `state_manager.h`
3. Add transition logic in `app.cpp`
4. Add UI files in `ui/` directory
5. **Important**: Add status icon updates in screen controller

#### Adding a New Driver
1. Create `drivers/new_driver.h/.cpp`
2. Initialize in `App::initialize()` in correct order
3. Access directly from screen controllers that need it
4. **Important**: Add health monitoring in `App::tick()`

#### Adding Business Logic
1. Add to `business/dictionary_api.h/.cpp`
2. Call from screen controllers
3. Keep business logic separate from UI and hardware

## Migration Strategy

### Phase 1: Remove Service Layer
- [ ] Remove all service classes
- [ ] Update screen controllers to access drivers directly
- [ ] Update App controller to initialize drivers directly
- [ ] Update State Manager to check driver health directly
- [ ] **Critical**: Add status icon updates to screen controllers
- [ ] **Critical**: Add health monitoring to App controller

### Phase 2: Simplify Dependencies
- [ ] Remove complex event system
- [ ] Use direct method calls
- [ ] Simplify initialization sequence

### Phase 3: Test and Refine
- [ ] Test all functionality
- [ ] Refine error handling
- [ ] Optimize performance
- [ ] **Critical**: Test status icon updates
- [ ] **Critical**: Test health monitoring and recovery

## Summary

This simplified architecture achieves:
- **60% reduction** in core classes
- **Direct communication** between components
- **Clear separation** of concerns
- **Easy to understand** for new coders
- **All functionality preserved**

The key insight is that **Screen Controllers** should handle UI logic and directly access **Drivers** for hardware operations, while the **App Controller** coordinates everything through the **State Manager**.

**Important Notes:**
- **Status Icons**: Must be manually updated by screen controllers
- **Health Monitoring**: Must be handled by App controller
- **Initialization Order**: Must be carefully managed by App controller
- **Error Recovery**: Must be handled by State Manager + App controller