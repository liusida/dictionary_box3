# Library Function Usage Summary

**Date:** January 27, 2025  
**Purpose:** Comprehensive overview of functions used from each library in the test suite

## Overview

This document summarizes the functions used from each library in the `./lib/` directory as tested in the `./test/` directory. The test suite provides comprehensive coverage of initialization, state management, event processing, and memory leak detection.

## Library Function Usage

### drivers_blekeyboard → test_drivers_blekeyboard

**BLEKeyboard Class:**
- `initialize()` - Initialize BLE keyboard connection
- `isConnected()` - Check if BLE keyboard is connected  
- `tick()` - Process BLE keyboard events
- `setKeyCallback()` - Set callback for key events
- `shutdown()` - Clean shutdown of BLE keyboard

**KeyProcessor Class:**
- `sendKeyToLVGL()` - Send key events to LVGL system
- `tick()` - Process queued key events

### drivers_audio → test_drivers_audio

**AudioManager Class:**
- `initialize()` - Initialize audio system
- `isReady()` - Check if audio system is ready
- `tick()` - Process audio events
- `setVolume(float)` - Set audio volume (0.0f to 1.0f)
- `play(const char*)` - Play audio from URL or local file
- `stop()` - Stop audio playback
- `isCurrentlyPlaying()` - Check if audio is playing
- `isCurrentlyPaused()` - Check if audio is paused
- `getCurrentUrl()` - Get current audio URL
- `shutdown()` - Clean shutdown of audio system

### drivers_display → test_drivers_display

**DisplayManager Class:**
- `initialize()` - Initialize display system
- `isReady()` - Check if display is ready
- `tick()` - Process display events
- `setBacklight(bool)` - Control display backlight
- `tickCallback()` - LVGL tick callback (static)
- `shutdown()` - Clean shutdown of display

**LVGL Helper Functions:**
- `getDefaultGroup()` - Get default LVGL group
- `loadScreen(lv_obj_t*)` - Load LVGL screen
- `addObjectToDefaultGroup(lv_obj_t*)` - Add objects to default group

### drivers_i2c → test_drivers_i2c

**I2CManager Class:**
- `instance()` - Get singleton instance
- `initialize()` - Initialize I2C bus
- `isReady()` - Check if I2C is ready
- `setFrequency(uint32_t)` - Set I2C frequency
- `getFrequency()` - Get current I2C frequency
- `getWire()` - Get TwoWire reference

### core_eventing → test_core_eventing

**EventSystem Class:**
- `instance()` - Get singleton event system
- `getEventBus<T>()` - Get event bus for type T
- `processAllEvents()` - Process all queued events

**EventBus<T> Class:**
- `subscribe()` - Subscribe to events
- `publish()` - Publish events
- `processEvents()` - Process queued events
- `unsubscribe()` - Unsubscribe from events
- `clear()` - Clear all listeners

**EventPublisher Class:**
- `instance()` - Get singleton publisher
- `publish<T>()` - Generic publish method

### core_misc → Used across multiple tests

**Memory Test Helper Functions:**
- `setUpMemoryMonitoring()` - Setup memory tracking
- `tearDownMemoryMonitoring()` - Check for memory leaks
- `printTestSuiteMemorySummary()` - Print memory summary
- `checkMemoryUsage()` - Check for memory leaks
- `RUN_TEST_EX()` - Enhanced test runner with memory monitoring

## Test Patterns

1. **Initialization/Shutdown Pattern**: Most tests follow `initialize() → test functionality → shutdown()`
2. **Memory Monitoring**: All tests use memory leak detection helpers
3. **State Checking**: Tests verify `isReady()` status before operations
4. **Tick Processing**: Many tests call `tick()` methods for event processing
5. **Event System Integration**: Tests use the event system for decoupled communication

## Additional Libraries Referenced

- **core_log** - Used for logging (ESP_LOGI, ESP_LOGE, etc.)
- **drivers_network** - Referenced in WiFi tests but not directly tested

## Coverage Summary

The test suite provides comprehensive coverage of:
- ✅ Initialization and shutdown procedures
- ✅ State management and readiness checks
- ✅ Event processing and tick methods
- ✅ Memory leak detection and monitoring
- ✅ Error handling and edge cases
- ✅ Integration between different library components

All major library functions are tested with proper initialization, state verification, and cleanup procedures.
