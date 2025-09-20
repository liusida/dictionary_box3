# Two-Tier Architecture & Runtime Failure Handling

**Date:** September 20, 2025

## Overview
Refactored Dictionary v2 to implement a two-tier architecture with robust runtime failure handling. Essential services start immediately while connectivity services initialize in background and can fail/recover gracefully during operation.

## Architecture Changes

### Two-Tier Service System
- **Tier 1 (Essential)**: DisplayManager, AudioManager, KeyProcessor - must work
- **Tier 2 (Optional)**: BLEKeyboard, WiFiControl - can fail gracefully

### New Service Classes
- `CoreServices` - Manages Tier 1 services (synchronous initialization)
- `ConnectivityServices` - Manages Tier 2 services (asynchronous initialization)  
- `ServiceManager` - Coordinates between tiers, replaces old `Services` class

## Key Features

### 1. Fast Startup
```
1. Initialize Tier 1 services (display, audio, input)
2. Show splash screen immediately
3. Start Tier 2 initialization in background
4. Transition to main when ready
```

### 2. Runtime Failure Handling
- **ConnectionMonitor** - Monitors service health every 2 seconds
- **RuntimeStateManager** - Handles automatic UI transitions
- **Graceful degradation** - Core features always available
- **Auto-recovery** - Shows settings screen when services fail

### 3. User Experience Flow
```
Main Screen (Normal)
    ↓ (WiFi fails)
WiFi Settings Screen (Auto-transition)
    ↓ (WiFi reconnects)
Main Screen (Auto-return)
```

## Implementation

### Core Services (`src/core/`)
- `core_services.h/cpp` - Tier 1 service management
- `connectivity_services.h/cpp` - Tier 2 service management
- `service_manager.h/cpp` - Unified service coordination
- `connection_monitor.h/cpp` - Health monitoring
- `runtime_state_manager.h/cpp` - UI state management

### Updated Files
- `src/main.cpp` - Tiered initialization flow
- `src/controllers/app.cpp` - Integrated monitoring
- `src/controllers/main_screen.h/cpp` - Connection status display

## Benefits
- ✅ **Faster startup** - Essential services start immediately
- ✅ **Better UX** - User sees splash while connectivity initializes
- ✅ **Resilient** - App works even when BLE/WiFi fail
- ✅ **Self-healing** - Automatic recovery with user guidance
- ✅ **Cleaner code** - Clear separation of concerns

## Testing Scenarios
1. **Normal operation** - All services connect successfully
2. **WiFi failure** - App shows WiFi settings, continues with limited features
3. **BLE failure** - App shows keyboard settings, continues with touch input
4. **Service recovery** - Automatic return to main when services reconnect
5. **Multiple failures** - Handles multiple service failures gracefully

The app now provides a robust, user-friendly experience that handles real-world connectivity challenges while keeping core functionality always available.
