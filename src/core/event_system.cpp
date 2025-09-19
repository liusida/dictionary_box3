#include "event_system.h"
#include "events.h"

EventSystem& EventSystem::instance() {
    static EventSystem instance;
    return instance;
}

void EventSystem::processAllEvents() {
    // Process all event types
    getEventBus<KeyEvent>().processEvents();
    getEventBus<FunctionKeyEvent>().processEvents();
    getEventBus<DictionaryEvent>().processEvents();
    getEventBus<AudioEvent>().processEvents();
    getEventBus<WiFiEvent>().processEvents();
    getEventBus<AppStateEvent>().processEvents();
}
