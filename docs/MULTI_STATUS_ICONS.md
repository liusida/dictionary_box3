# Multi-Status Icon System

This document describes the enhanced UI status icon system that supports multiple status indicators for WiFi, BLE keyboard, audio playback, and waiting states.

## Overview

The new system extends the original single `UIStatusIcon` to support four different status indicators positioned in different corners of the screen. **These icons are global system indicators that are visible on ALL screens** (splash, main, settings, etc.):

- **WiFi Icon** (Top-left): Shows WiFi connection status with lookup animation
- **BLE Icon** (Top-center): Shows BLE keyboard connection status with key notification animation
- **Audio Icon** (Top-right): Shows audio playback status with playing animation
- **Waiting Icon** (Bottom-right): Shows waiting/processing status with manual control

### Global Visibility

The status icons are attached to LVGL's global top layer (`lv_layer_top()`) and are automatically visible on all screens. They are initialized early in the application lifecycle and refresh automatically when screen transitions occur.

## Icon Positions and Colors

| Icon Type | Position | Color | Stable State | Spinning State | Disconnected State |
|-----------|----------|-------|--------------|----------------|-------------------|
| WiFi      | Top-left | Green (#2da041) | Connected | During lookup | Red (#d32f2f) |
| BLE       | Top-center | Blue (#2095f6) | Connected | Key notification | Red (#d32f2f) |
| Audio     | Top-right | Orange (#ff9800) | Ready | Playing | Hidden |
| Waiting   | Bottom-right | Grey (#757575) | Stable | Processing | Hidden |

## Usage Examples

### WiFi Status (Green Icon)
```cpp
// Show WiFi connected (green stable icon)
UIStatusIcon::instance().showWiFiConnected();

// Show WiFi disconnected (red stable icon)
UIStatusIcon::instance().showWiFiDisconnected();

// Show WiFi lookup in progress (green spinning icon)
UIStatusIcon::instance().showWiFiLookup();

// Hide WiFi icon
UIStatusIcon::instance().hideWiFi();
```

### BLE Keyboard Status (Blue Icon)
```cpp
// Show BLE connected (blue stable icon)
UIStatusIcon::instance().showBLEConnected();

// Show BLE disconnected (red stable icon)
UIStatusIcon::instance().showBLEDisconnected();

// Show BLE key notification (blue spinning icon)
UIStatusIcon::instance().showBLEKeyNotification();

// Hide BLE icon
UIStatusIcon::instance().hideBLE();
```

### Audio Status (Orange Icon)
```cpp
// Show audio ready (orange stable icon)
UIStatusIcon::instance().showAudioReady();

// Show audio playing (orange spinning icon)
UIStatusIcon::instance().showAudioPlaying();

// Hide audio icon
UIStatusIcon::instance().hideAudio();
```

### Waiting Status (Grey Icon - Manual Control)
```cpp
// Show waiting stable (grey stable icon)
UIStatusIcon::instance().showWaitingStable();

// Show waiting/processing (grey spinning icon)
UIStatusIcon::instance().showWaiting();

// Hide waiting icon
UIStatusIcon::instance().hideWaiting();
```

## Automatic Status Updates

The system includes automatic status monitoring through the `ConnectionStatusUpdater` class:

- **WiFi Status**: Automatically updated based on `ConnectionMonitor::isWiFiHealthy()`
  - Shows green stable when connected, hidden when disconnected
  - Shows green spinning during dictionary lookups
- **BLE Status**: Automatically updated based on `ConnectionMonitor::isBLEHealthy()`
  - Shows blue stable when connected, hidden when disconnected
  - Shows blue spinning during key notifications
- **Audio Status**: Automatically updated based on `AudioEvent` events
  - Shows orange stable when ready, orange spinning when playing
- **Waiting Status**: Manual control for general waiting/processing states
  - Shows grey stable or grey spinning as needed

## Integration Points

### Global Initialization
The status icons are initialized early in the application lifecycle in `src/controllers/app.cpp`:

```cpp
bool App::initialize() {
    // ... other initialization ...
    
    // Initialize global status icons (must be done early for global visibility)
    UIStatusIcon::instance().initialize();
    
    // Initialize connection status updater
    ConnectionStatusUpdater::instance().initialize();
}
```

### Screen Transitions
Icons are refreshed on every screen transition to ensure visibility:

```cpp
void App::enterMainState() {
    // ... screen setup ...
    
    // Ensure status icons are visible on main screen
    UIStatusIcon::instance().refreshForCurrentScreen();
}
```

### Main Application Loop
The `ConnectionStatusUpdater` is integrated into the main application loop:

```cpp
void App::tick() {
    // ... other tick calls ...
    
    // Update connection status icons
    ConnectionStatusUpdater::instance().tick();
}
```

### Event-Driven Updates
Status icons are automatically updated based on system events:

- **Dictionary Lookups**: WiFi icon spins during word lookups, returns to stable when complete
- **Audio Playback**: Audio icon spins during playback, shows stable when ready
- **BLE Key Events**: BLE icon spins briefly during key notifications
- **Connection Changes**: WiFi/BLE icons update when connections change

## Legacy Compatibility

The system maintains backward compatibility with the original API:

```cpp
// Legacy methods still work (map to audio status)
UIStatusIcon::instance().showBlue();   // Shows audio playing (orange spinning)
UIStatusIcon::instance().showGreen();  // Shows audio ready (orange stable)
UIStatusIcon::instance().hide();      // Hides audio
```

## Testing

A test file is provided at `src/core/status_icon_test.cpp` that demonstrates all functionality:

```cpp
// Run the demo
StatusIconTest::runDemo();

// Test legacy compatibility
StatusIconTest::testLegacyCompatibility();

// Test global visibility across screens
StatusIconTest::testGlobalVisibility();
```

## Technical Implementation

### Icon Creation
Icons are created early during application initialization and attached to `lv_layer_top()` for global visibility across all screens.

### Positioning
Each icon type has a predefined position using LVGL alignment and offset values.

### Styling
Icons use LVGL spinner objects with different colors and animation states:
- Static icons for WiFi/BLE/Audio status
- Animated spinner for loading status

### Memory Management
All icons are properly cleaned up in the destructor to prevent memory leaks.

## Files Modified/Created

### Modified Files
- `src/core/ui_status_icon.h` - Extended interface
- `src/core/ui_status_icon.cpp` - New implementation
- `src/controllers/main_screen.cpp` - Updated to use new audio/loading methods
- `src/controllers/main_screen.h` - Added dictionary event handler
- `src/controllers/app.cpp` - Integrated connection status updater

### New Files
- `src/core/connection_status_updater.h` - Automatic status monitoring
- `src/core/connection_status_updater.cpp` - Implementation
- `src/core/status_icon_test.cpp` - Test/demo functionality
- `MULTI_STATUS_ICONS.md` - This documentation

## Future Enhancements

Potential future improvements:
- Custom icon shapes instead of spinners
- Configurable positions
- Additional status types (battery, storage, etc.)
- Icon size customization
- Animation effects for state transitions
