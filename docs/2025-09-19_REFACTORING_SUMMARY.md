# Dictionary v2 Refactoring Summary

**Date:** September 19, 2025

## Overview
This document summarizes the major refactoring changes made to improve the architecture, reduce coupling, and create cleaner interfaces between modules.

## Key Improvements

### 1. Core Services Layer (`src/core/`)
- **`services.h/cpp`**: Centralized service registry that manages all system services
- **`driver_interface.h`**: Standardized interface for all drivers
- **`event_system.h/cpp`**: Event bus system for decoupled communication
- **`event_publisher.h/cpp`**: Helper for publishing events
- **`events.h`**: Event type definitions

### 2. Service Layer (`src/services/`)
- **`dictionary_service.h/cpp`**: Extracted HTTP and audio logic from UI controllers
- Handles all dictionary API communication and audio streaming
- Clean separation of business logic from UI logic

### 3. Controller Layer (`src/controllers/`)
- **`app_controller.h/cpp`**: Main application state management
- **`main_screen_controller.h/cpp`**: New refactored main screen controller
- **`main_screen_control.h/cpp`**: Legacy main screen control (moved from src/)
- **`splash_controller.h/cpp`**: Splash screen controller
- **`keyboard_settings_controller.h/cpp`**: Keyboard settings controller
- **`wifi_settings_controller.h/cpp`**: WiFi settings controller
- Separated UI logic from business logic and protected from UI overwrites

### 4. Input Layer (`src/input/`)
- **`key_processor.h/cpp`**: Standardized key processing with event support
- Replaces the old `key_processing.cpp` with cleaner interface

### 5. Driver Layer (`src/drivers/`)
- **`display_manager.h/cpp`**: Wrapper for display and touch functionality
- Updated all drivers to implement `DriverInterface`
- Consistent lifecycle management across all drivers

## Architecture Changes

### Before (Problems)
- Global objects scattered across files
- Tight coupling between modules
- Mixed interface patterns (C-style functions vs classes)
- Circular dependencies
- Mixed responsibilities in single files
- No clear ownership or lifecycle management

### After (Solutions)
- Centralized service registry manages all services
- Clean interfaces with `DriverInterface` base class
- Event-driven communication for loose coupling
- Clear separation of concerns
- Consistent patterns across all modules
- Proper lifecycle management

## Key Benefits

1. **Reduced Coupling**: Modules communicate through events and services
2. **Clear Interfaces**: All drivers implement consistent `DriverInterface`
3. **Better Testability**: Services can be easily mocked and tested
4. **Maintainability**: Clear separation of concerns makes code easier to understand
5. **Extensibility**: New features can be added without affecting existing code
6. **Error Handling**: Centralized error handling through events
7. **UI Protection**: Custom controller logic protected from SquareLine Studio overwrites
8. **Organized Structure**: Each screen has dedicated controller for better organization
9. **Stack Safety**: Queued event system prevents stack overflow from interrupt callbacks

## Critical Architectural Principles

### ⚠️ **STACK SAFETY - NEVER CALL HEAVY OPERATIONS FROM INTERRUPT CALLBACKS**

**Problem**: Interrupt callbacks (BLE, WiFi, etc.) run in limited stack contexts and can cause stack overflow if they trigger heavy operations like:
- SSL/TLS handshakes (audio streaming, HTTPS requests)
- Complex UI operations
- File I/O operations
- Memory allocations

**Solution**: Use the **queued event system**:
```cpp
// ❌ WRONG - Direct call in BLE callback (causes stack overflow)
void BLEKeyboard::notifyCB(...) {
    audioManager.play(url);  // SSL handshake needs lots of stack!
}

// ✅ CORRECT - Queue event for later processing
void BLEKeyboard::notifyCB(...) {
    KeyEvent event{key, keyCode, modifiers, true};
    EventSystem::instance().getEventBus<KeyEvent>().publish(event);  // Just queues
}

// In main loop - process events safely
void AppController::tick() {
    EventSystem::instance().processAllEvents();  // Safe processing
}
```

**Key Rules**:
1. **Interrupt callbacks**: Only queue events, never call heavy operations
2. **Event processing**: Only in main loop or dedicated tasks with sufficient stack
3. **Audio/Network**: Always defer to main loop via events
4. **UI updates**: Use LVGL task or main loop, never from interrupts

## Migration Guide

### For Existing Code
- Replace direct global object access with `Services::instance().serviceName()`
- Use event subscriptions instead of direct function calls
- Implement `DriverInterface` for new drivers

### For New Features
- Create services in the `services/` directory
- Use controllers for UI logic
- Publish events for important state changes
- Subscribe to events for reactive behavior

## File Structure
```
src/
├── ui/                              # Auto-generated UI files (protected from overwrites)
│   ├── ui_Main.c/h                  # Main screen UI
│   ├── ui_Splash.c/h                # Splash screen UI  
│   ├── ui_Keyboard_Settings.c/h     # Keyboard settings UI
│   └── ui_WIFI_Settings.c/h         # WiFi settings UI
├── controllers/                     # Custom controller logic (protected from UI overwrites)
│   ├── app_controller.h/cpp         # Main application controller
│   ├── main_screen_controller.h/cpp # New refactored main screen controller
│   ├── main_screen_control.h/cpp    # Legacy main screen control
│   ├── splash_controller.h/cpp      # Splash screen controller
│   ├── keyboard_settings_controller.h/cpp  # Keyboard settings controller
│   └── wifi_settings_controller.h/cpp      # WiFi settings controller
├── core/                    # Core infrastructure
│   ├── driver_interface.h   # Base interface for drivers
│   ├── event_system.h       # Event bus system
│   ├── event_publisher.h    # Event publishing helper
│   ├── events.h            # Event type definitions
│   └── services.h          # Service registry
├── services/               # Business logic services
│   └── dictionary_service.h
├── input/                  # Input handling
│   └── key_processor.h
├── drivers/                # Hardware drivers
│   ├── display_manager.h
│   ├── audio_manager.h
│   ├── ble_keyboard.h
│   └── wifi_control.h
└── main.cpp               # Application entry point
```

## Usage Examples

### Accessing Services
```cpp
// Old way
extern AudioManager audio;
audio.play(url);

// New way
Services::instance().audio().play(url);
```

### Publishing Events
```cpp
// Publish an event
EventPublisher::instance().publishAudioEvent(AudioEvent::PlaybackStarted, url);
```

### Subscribing to Events
```cpp
// Subscribe to events
services_.keyProcessor().onFunctionKeyEvent(
    [this](const FunctionKeyEvent& event) {
        handleFunctionKey(event);
    }
);
```

## Future Improvements
1. Add more event types as needed
2. Implement service discovery
3. Add configuration management service
4. Implement logging service
5. Add metrics and monitoring

This refactoring provides a solid foundation for future development while maintaining backward compatibility with existing code.
