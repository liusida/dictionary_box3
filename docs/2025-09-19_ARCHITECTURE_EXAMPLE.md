# Architecture Usage Examples

**Date:** September 19, 2025

## Service Access
```cpp
#include "core/services.h"

void someFunction() {
    auto& audio = Services::instance().audio();
    auto& wifi = Services::instance().wifi();
    
    if (wifi.isConnected()) {
        audio.play("http://example.com/audio.mp3");
    }
}
```

## Event-Driven Communication
```cpp
#include "core/event_system.h"
#include "core/events.h"

class MyController {
public:
    void initialize() {
        // Subscribe to events
        EventSystem::instance().getEventBus<AudioEvent>().subscribe(
            [this](const AudioEvent& event) { onAudioEvent(event); }
        );
    }
    
private:
    void onAudioEvent(const AudioEvent& event) {
        switch (event.type) {
            case AudioEvent::PlaybackStarted:
                ESP_LOGI("Controller", "Audio started: %s", event.url.c_str());
                break;
        }
    }
};
```

## Driver Implementation
```cpp
#include "core/driver_interface.h"

class MyDriver : public DriverInterface {
public:
    bool initialize() override {
        // Initialize hardware
        initialized_ = true;
        return true;
    }
    
    void shutdown() override {
        // Cleanup
        initialized_ = false;
    }
    
    void tick() override {
        // Called every loop iteration
    }
    
    bool isReady() const override {
        return initialized_;
    }
    
private:
    bool initialized_ = false;
};
```

## Event Publishing
```cpp
#include "core/event_publisher.h"

// Publish events
EventPublisher::instance().publishAudioEvent(AudioEvent::PlaybackStarted, url);
EventPublisher::instance().publishWiFiEvent(WiFiEvent::Connected);
```

## Event Types
- **AudioEvent**: `PlaybackStarted`, `PlaybackStopped`, `PlaybackError`
- **WiFiEvent**: `Connected`, `Disconnected`, `ConnectionFailed`
- **KeyEvent**: Key press data with modifiers
- **FunctionKeyEvent**: `VolumeUp`, `VolumeDown`, `ReadWord`, etc.
- **DictionaryEvent**: `LookupStarted`, `LookupCompleted`, `LookupFailed`
- **AppStateEvent**: `EnteringSplash`, `EnteringMain`, `SystemReady`

## Best Practices
1. **Service Access**: Use `Services::instance().serviceName()` instead of globals
2. **Event Handling**: Subscribe in `initialize()`, unsubscribe in `shutdown()`
3. **Driver Implementation**: Implement all `DriverInterface` methods
4. **Error Handling**: Publish error events for important failures
5. **Separation**: Keep UI logic in controllers, business logic in services
6. **UI Organization**: Place custom controller logic in `./src/controllers/` to protect from SquareLine Studio overwrites
7. **Screen Controllers**: Create dedicated controllers for each screen (main, splash, settings, etc.)
8. **⚠️ CRITICAL - Stack Safety**: Never call heavy operations from interrupt callbacks - use queued events!

### Stack Safety Rules
```cpp
// ❌ WRONG - Causes stack overflow
void BLEKeyboard::notifyCB(...) {
    Services::instance().audio().play(url);  // SSL needs lots of stack!
}

// ✅ CORRECT - Queue event for safe processing
void BLEKeyboard::notifyCB(...) {
    KeyEvent event{key, keyCode, modifiers, true};
    EventSystem::instance().getEventBus<KeyEvent>().publish(event);
}
```

## Migration from Old Code
```cpp
// Old way
extern AudioManager audio;
audio.play(url);

// New way
Services::instance().audio().play(url);
```

**Result**: Clean, event-driven architecture with consistent patterns and reduced coupling.