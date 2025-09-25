# Redesigned Multi-Status Icon System

## Overview

The status icon system has been redesigned according to your specifications with a cleaner, more intuitive design that uses stable/spinning states for each icon type.

## New Design Philosophy

Each icon type has a consistent color and uses animation (spinning) to indicate active states:

### 🟢 WiFi Icon (Top-left, Green)
- **Stable**: Shows when WiFi is connected
- **Spinning**: Shows during dictionary lookups
- **Hidden**: When WiFi is disconnected

### 🔵 BLE Icon (Top-center, Blue)  
- **Stable**: Shows when BLE keyboard is connected
- **Spinning**: Shows briefly during key notifications
- **Hidden**: When BLE is disconnected

### 🟠 Audio Icon (Top-right, Orange)
- **Stable**: Shows when audio is ready
- **Spinning**: Shows during audio playback
- **Hidden**: When audio is not available

### ⚫ Waiting Icon (Bottom-right, Grey)
- **Stable**: Manual control for general waiting states
- **Spinning**: Manual control for active processing
- **Hidden**: When not needed

## Key Changes Made

### 1. Simplified Status Enum
```cpp
enum class Status {
    Hidden,
    Stable,       // Static icon
    Spinning      // Animated icon
};
```

### 2. Icon-Specific Colors
- WiFi: Green (#2da041)
- BLE: Blue (#2095f6) 
- Audio: Orange (#ff9800)
- Waiting: Grey (#757575)

### 3. New Method Names
```cpp
// WiFi methods
showWiFiConnected()      // Green stable
showWiFiLookup()         // Green spinning

// BLE methods  
showBLEConnected()       // Blue stable
showBLEKeyNotification() // Blue spinning

// Audio methods
showAudioReady()         // Orange stable
showAudioPlaying()       // Orange spinning

// Waiting methods (manual control)
showWaitingStable()      // Grey stable
showWaiting()            // Grey spinning
```

### 4. Automatic Behaviors

**WiFi Icon:**
- Automatically shows green stable when connected
- Automatically spins during dictionary lookups
- Automatically returns to stable when lookup completes

**BLE Icon:**
- Automatically shows blue stable when connected  
- Automatically spins briefly during key notifications
- Automatically returns to stable after key event

**Audio Icon:**
- Automatically shows orange stable when ready
- Automatically spins during audio playback
- Automatically returns to stable when playback stops

**Waiting Icon:**
- Manual control for any waiting/processing states
- Use `showWaiting()` for spinning animation
- Use `showWaitingStable()` for static waiting indicator

## Integration Points

### Dictionary Lookups
```cpp
// In onDictionaryEvent()
case DictionaryEvent::LookupStarted:
    UIStatusIcon::instance().showWiFiLookup();  // Green spinning
    break;
case DictionaryEvent::LookupCompleted:
    UIStatusIcon::instance().showWiFiConnected(); // Green stable
    break;
```

### Audio Playback
```cpp
// In onAudioEvent()
case AudioEvent::PlaybackStarted:
    UIStatusIcon::instance().showAudioPlaying(); // Orange spinning
    break;
case AudioEvent::PlaybackStopped:
    UIStatusIcon::instance().showAudioReady();   // Orange stable
    break;
```

### BLE Key Events
```cpp
// In onFunctionKeyEvent()
UIStatusIcon::instance().showBLEKeyNotification(); // Blue spinning
// ... handle key event ...
UIStatusIcon::instance().showBLEConnected();        // Blue stable
```

### Manual Waiting States
```cpp
// Show waiting animation
UIStatusIcon::instance().showWaiting();

// Show stable waiting indicator  
UIStatusIcon::instance().showWaitingStable();

// Hide waiting indicator
UIStatusIcon::instance().hideWaiting();
```

## Benefits of New Design

1. **Consistent Visual Language**: Each icon type has a unique color and consistent stable/spinning behavior
2. **Intuitive Animation**: Spinning indicates active/processing states
3. **Cleaner API**: Simplified method names that clearly indicate the intended behavior
4. **Better UX**: Users can quickly understand what's happening based on icon color and animation
5. **Flexible Waiting**: Manual control over waiting states for any custom processing needs

## Files Modified

- `src/core/ui_status_icon.h` & `.cpp` - Complete redesign
- `src/core/connection_status_updater.cpp` - Updated for new WiFi/BLE behavior
- `src/controllers/main_screen.cpp` - Updated event handlers
- `src/core/status_icon_test.cpp` - Updated test examples
- `MULTI_STATUS_ICONS.md` - Updated documentation

The system is now ready with your redesigned specifications!
