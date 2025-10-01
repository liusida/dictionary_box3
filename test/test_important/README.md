# Test Suite: Interfaces Used by Src

This test suite focuses on testing the most important interfaces from `./lib/` that are used in `./src/`. These are the core functions that the main application logic depends on, so they need thorough testing.

## Test Files

### `test_dictionary_api.cpp`
Tests the DictionaryApi class functionality used in src:
- `initialize()` and `isReady()` methods
- `lookupWord()` functionality
- `getAudioUrl()` for different audio types
- `prewarm()` method
- Word validation and error handling

### `test_display_manager.cpp`
Tests the DisplayManager singleton functionality used in src:
- `initialize()` and `isReady()` methods
- `tick()` safety and stability
- `resetDisplay()` functionality
- `setBacklight()` control

### `test_audio_manager.cpp`
Tests the AudioManager singleton functionality used in src:
- `initialize()` and `isReady()` methods
- `tick()` safety and stability
- `play()` and `stop()` functionality
- `setVolume()` and `getVolume()` control

### `test_ble_keyboard.cpp`
Tests the BLEKeyboard singleton functionality used in src:
- `initialize()` and `isReady()` methods
- `tick()` safety and stability
- `startScan()` and scanning state management
- `isConnected()` and connection state

### `test_network_control.cpp`
Tests the NetworkControl singleton functionality used in src:
- `initialize()` and `isReady()` methods
- `tick()` safety and stability
- Connection state management
- Credential management (save/load/clear)
- Network scanning functionality

### `test_status_overlay.cpp`
Tests the StatusOverlay singleton functionality used in src:
- `initialize()` and `isReady()` methods
- `attachToScreen()` and `detachFromScreen()` functionality
- Status update methods for WiFi, BLE, and Audio
- Position and style control

### `test_event_system.cpp`
Tests the EventSystem functionality used in src:
- `processAllEvents()` method
- FunctionKeyEvent handling
- Event bus management and subscription

### `test_lvgl_helper.cpp`
Tests the LVGL helper functions used in src:
- Key callback management
- Function key callback management
- Group management functions

## Running the Tests

This test suite can be run using PlatformIO:

```bash
pio test -e dictionary --filter test_interfaces_used_by_src
```

## Test Philosophy

These tests focus on:
1. **Safety**: Ensuring methods don't crash when called
2. **State Management**: Verifying proper initialization, ready states, and shutdown
3. **Error Handling**: Testing graceful handling of invalid inputs
4. **Core Functionality**: Testing the essential methods used by the main application

The tests are designed to be robust and not require external hardware or network connections where possible, making them suitable for continuous integration.
