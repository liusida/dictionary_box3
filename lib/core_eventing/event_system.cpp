#include "event_system.h"
#include "events.h"
#include <mutex>

namespace dict {

EventSystem& EventSystem::instance() {
    static EventSystem instance;
    return instance;
}

void EventSystem::processAllEvents() {
    // Process built-in event types (for backward compatibility)
    getEventBus<KeyEvent>().processEvents();
    getEventBus<FunctionKeyEvent>().processEvents();
    getEventBus<DictionaryEvent>().processEvents();
    getEventBus<AudioEvent>().processEvents();
    getEventBus<WiFiEvent>().processEvents();
    getEventBus<AppStateEvent>().processEvents();
    getEventBus<UIEvent>().processEvents();

    // Process any registered custom event buses
    {
        std::lock_guard<std::mutex> lock(processorsMutex_);
        for (auto &proc : processors_) {
            proc();
        }
    }
}

} // namespace dict

// Explicit template instantiations to ensure one bus per type across TUs
template dict::EventBus<dict::KeyEvent>& dict::EventSystem::getEventBus<dict::KeyEvent>();
template dict::EventBus<dict::FunctionKeyEvent>& dict::EventSystem::getEventBus<dict::FunctionKeyEvent>();
template dict::EventBus<dict::DictionaryEvent>& dict::EventSystem::getEventBus<dict::DictionaryEvent>();
template dict::EventBus<dict::AudioEvent>& dict::EventSystem::getEventBus<dict::AudioEvent>();
template dict::EventBus<dict::WiFiEvent>& dict::EventSystem::getEventBus<dict::WiFiEvent>();
template dict::EventBus<dict::AppStateEvent>& dict::EventSystem::getEventBus<dict::AppStateEvent>();
template dict::EventBus<dict::UIEvent>& dict::EventSystem::getEventBus<dict::UIEvent>();
