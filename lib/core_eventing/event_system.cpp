#include "event_system.h"
#include "events.h"

namespace core { namespace eventing {

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

}} // namespace core::eventing

// Explicit template instantiations to ensure one bus per type across TUs
template core::eventing::EventBus<core::eventing::KeyEvent>& core::eventing::EventSystem::getEventBus<core::eventing::KeyEvent>();
template core::eventing::EventBus<core::eventing::FunctionKeyEvent>& core::eventing::EventSystem::getEventBus<core::eventing::FunctionKeyEvent>();
template core::eventing::EventBus<core::eventing::DictionaryEvent>& core::eventing::EventSystem::getEventBus<core::eventing::DictionaryEvent>();
template core::eventing::EventBus<core::eventing::AudioEvent>& core::eventing::EventSystem::getEventBus<core::eventing::AudioEvent>();
template core::eventing::EventBus<core::eventing::WiFiEvent>& core::eventing::EventSystem::getEventBus<core::eventing::WiFiEvent>();
template core::eventing::EventBus<core::eventing::AppStateEvent>& core::eventing::EventSystem::getEventBus<core::eventing::AppStateEvent>();
template core::eventing::EventBus<core::eventing::UIEvent>& core::eventing::EventSystem::getEventBus<core::eventing::UIEvent>();
