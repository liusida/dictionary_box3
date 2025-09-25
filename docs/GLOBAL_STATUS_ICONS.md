# Global Status Icons System

## Overview

The status icons are now **truly global system indicators** that are visible on ALL screens throughout the application. They provide real-time status information regardless of which screen the user is currently viewing.

## Key Features

### 🌐 **Global Visibility**
- Icons are visible on **all screens**: splash, main, WiFi settings, keyboard settings, etc.
- Attached to LVGL's global top layer (`lv_layer_top()`)
- Automatically refresh when screen transitions occur
- Initialized early in application lifecycle

### 🎯 **Icon Layout**
- **WiFi** (Top-left, Green): Connection status + lookup animation
- **BLE** (Top-center, Blue): Keyboard connection + key notification animation  
- **Audio** (Top-right, Orange): Audio ready + playback animation
- **Waiting** (Bottom-right, Grey): Manual control for any waiting states

### 🔄 **Automatic Behaviors**
- **WiFi**: Green stable when connected, spinning during lookups
- **BLE**: Blue stable when connected, spinning during key events
- **Audio**: Orange stable when ready, spinning during playback
- **Waiting**: Manual control for custom waiting/processing states

## Implementation Details

### Early Initialization
```cpp
// In App::initialize()
UIStatusIcon::instance().initialize();  // Creates all icons early
ConnectionStatusUpdater::instance().initialize();  // Sets up automatic updates
```

### Screen Transition Handling
```cpp
// In each screen transition method
void App::enterMainState() {
    mainScreen_.enterMainState();
    UIStatusIcon::instance().refreshForCurrentScreen();  // Ensure visibility
}
```

### Global Layer Attachment
```cpp
// Icons are created on global top layer
lv_obj_t* parent = lv_layer_top();
lv_obj_t* icon = lv_spinner_create(parent);

// Additional properties for global visibility
lv_obj_move_foreground(icon);
lv_obj_clear_flag(icon, LV_OBJ_FLAG_SCROLLABLE);
```

## Usage Examples

### From Any Screen
```cpp
// These work from ANY screen in the application
UIStatusIcon::instance().showWiFiConnected();   // Green stable
UIStatusIcon::instance().showWiFiLookup();       // Green spinning
UIStatusIcon::instance().showBLEConnected();     // Blue stable
UIStatusIcon::instance().showBLEKeyNotification(); // Blue spinning
UIStatusIcon::instance().showAudioReady();       // Orange stable
UIStatusIcon::instance().showAudioPlaying();     // Orange spinning
UIStatusIcon::instance().showWaiting();          // Grey spinning
UIStatusIcon::instance().showWaitingStable();    // Grey stable
```

### Automatic Updates
The icons automatically update based on system events:
- WiFi connection changes
- BLE keyboard connection changes
- Dictionary lookup events
- Audio playback events
- BLE key notifications

## Benefits

1. **Consistent User Experience**: Status information is always visible regardless of current screen
2. **Real-time Feedback**: Users can see connection status, audio state, and processing indicators at all times
3. **System-wide Awareness**: No need to navigate to specific screens to check system status
4. **Clean Architecture**: Icons are truly standalone components with no screen dependencies

## Testing

```cpp
// Test global visibility
StatusIconTest::testGlobalVisibility();

// Test all functionality
StatusIconTest::runDemo();

// Test legacy compatibility
StatusIconTest::testLegacyCompatibility();
```

## Files Modified

- `src/core/ui_status_icon.h` & `.cpp` - Added global initialization and refresh methods
- `src/controllers/app.cpp` - Added early initialization and screen transition handling
- `src/core/status_icon_test.cpp` - Added global visibility test
- `MULTI_STATUS_ICONS.md` - Updated documentation

The status icons are now truly global system indicators that provide consistent, real-time status information across the entire application!
