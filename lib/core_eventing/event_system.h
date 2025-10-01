#pragma once
#include "common.h"
#include "events.h"
#include "psram_allocator.h"
#include <functional>
#include <mutex>
#include <queue>
#include <vector>

namespace dict {

/**
 * @brief Simple event bus for decoupled communication between modules
 *
 * This provides a publish-subscribe pattern for loose coupling between
 * different parts of the system.
 */
template <typename T> class EventBus {
public:
  using Listener = std::function<void(const T &)>;
  using ListenerId = size_t;

  /**
   * @brief Subscribe to events of type T
   * @param listener Function to call when event is published
   * @return Unique listener ID for unsubscribing
   */
  ListenerId subscribe(Listener listener) {
    listeners_.push_back(listener);
    return listeners_.size() - 1;
  }

  /**
   * @brief Publish an event to all subscribers (queued for later processing)
   * @param event The event to publish
   */
  void publish(const T &event) {
    std::lock_guard<std::mutex> lock(eventQueueMutex_);
    eventQueue_.push(event);
  }

  /**
   * @brief Process all queued events (call from main loop)
   */
  void processEvents() {
    std::queue<T, std::deque<T>> eventsToProcess;

    // Move events from queue to local queue (minimize lock time)
    {
      std::lock_guard<std::mutex> lock(eventQueueMutex_);
      eventsToProcess = std::move(eventQueue_);
    }

    // Process events outside of lock
    while (!eventsToProcess.empty()) {
      const T &event = eventsToProcess.front();
      for (const auto &listener : listeners_) {
        if (listener) {
          listener(event);
        }
      }
      eventsToProcess.pop();
    }
  }

  /**
   * @brief Unsubscribe a listener
   * @param id The listener ID returned by subscribe()
   */
  void unsubscribe(ListenerId id) {
    if (id < listeners_.size()) {
      listeners_[id] = nullptr;
    }
  }

  /**
   * @brief Clear all listeners
   */
  void clear() { listeners_.clear(); }

private:
  std::vector<Listener> listeners_;
  std::queue<T, std::deque<T>> eventQueue_;
  std::mutex eventQueueMutex_;
};

/**
 * @brief Global event bus manager
 *
 * Provides access to all event buses in the system
 */
class EventSystem {
public:
  static EventSystem &instance();

  // Event bus accessors
  template <typename T> EventBus<T> &getEventBus() {
    static EventBus<T> bus;
    return bus;
  }

  // Register an event bus type for inclusion in processAllEvents()
  template <typename T> void registerEventBus() {
    static bool registered = false;
    if (registered) {
      return;
    }
    {
      std::lock_guard<std::mutex> lock(processorsMutex_);
      processors_.push_back([]() { EventSystem::instance().getEventBus<T>().processEvents(); });
      registered = true;
    }
  }

  /**
   * @brief Process all queued events (call from main loop)
   */
  void processAllEvents();

private:
  EventSystem() = default;

  // Additional processors registered by users for custom event types
  std::vector<std::function<void()>, PsramAllocator<std::function<void()>>> processors_;
  std::mutex processorsMutex_;
};

// Ensure a single EventBus<T> instance per type across translation units
extern template EventBus<KeyEvent> &EventSystem::getEventBus<KeyEvent>();
extern template EventBus<FunctionKeyEvent> &EventSystem::getEventBus<FunctionKeyEvent>();

} // namespace dict
