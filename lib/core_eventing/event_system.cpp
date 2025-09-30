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
