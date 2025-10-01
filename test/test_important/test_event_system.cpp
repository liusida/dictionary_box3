#include <Arduino.h>
#include <unity.h>
#include "../../lib/core_eventing/event_system.h"
#include "../../lib/core_eventing/events.h"
#include "../../lib/core_misc/memory_test_helper.h"

using namespace dict;

// =================================== TESTS ===================================

void test_event_system_process_all_events(void) {
    EventSystem& eventSystem = EventSystem::instance();
    TEST_ASSERT_NOT_NULL_MESSAGE(&eventSystem, "EventSystem singleton should not be null");
    
    // Test processAllEvents (should not crash even with no events)
    eventSystem.processAllEvents();
    eventSystem.processAllEvents();
    eventSystem.processAllEvents();
    
    // Test with some events
    auto& keyEventBus = eventSystem.getEventBus<KeyEvent>();
    auto& functionKeyEventBus = eventSystem.getEventBus<FunctionKeyEvent>();
    
    // Publish some test events
    KeyEvent keyEvent;
    keyEvent.key = 'a';
    keyEvent.keyCode = 4;
    keyEvent.modifiers = 0;
    keyEvent.valid = true;
    keyEventBus.publish(keyEvent);
    
    FunctionKeyEvent functionKeyEvent(FunctionKeyEvent::ReadWord);
    functionKeyEventBus.publish(functionKeyEvent);
    
    // Process events
    eventSystem.processAllEvents();
    
    // Should not crash
    TEST_ASSERT_TRUE(true);
}

void test_event_system_function_key_events(void) {
    EventSystem& eventSystem = EventSystem::instance();
    auto& functionKeyEventBus = eventSystem.getEventBus<FunctionKeyEvent>();
    
    // Test event subscription and processing
    bool eventReceived = false;
    FunctionKeyEvent receivedEvent;
    
    auto listenerId = functionKeyEventBus.subscribe([&](const FunctionKeyEvent& event) {
        eventReceived = true;
        receivedEvent = event;
    });
    
    // Publish a function key event
    FunctionKeyEvent testEvent(FunctionKeyEvent::WifiSettings);
    functionKeyEventBus.publish(testEvent);
    
    // Process events
    eventSystem.processAllEvents();
    
    // Verify event was received
    TEST_ASSERT_TRUE_MESSAGE(eventReceived, "Function key event should be received");
    TEST_ASSERT_EQUAL_MESSAGE(FunctionKeyEvent::WifiSettings, receivedEvent.type, "Event type should match");
    
    // Test unsubscribe
    functionKeyEventBus.unsubscribe(listenerId);
    
    // Publish another event
    FunctionKeyEvent testEvent2(FunctionKeyEvent::ReadWord);
    functionKeyEventBus.publish(testEvent2);
    
    // Process events
    eventSystem.processAllEvents();
    
    // Event should not be received after unsubscribe
    TEST_ASSERT_EQUAL_MESSAGE(FunctionKeyEvent::WifiSettings, receivedEvent.type, "Event type should not change after unsubscribe");
}

void test_event_system_event_bus_management(void) {
    EventSystem& eventSystem = EventSystem::instance();
    
    // Test getting different event buses
    auto& keyEventBus = eventSystem.getEventBus<KeyEvent>();
    auto& functionKeyEventBus = eventSystem.getEventBus<FunctionKeyEvent>();
    
    TEST_ASSERT_NOT_NULL_MESSAGE(&keyEventBus, "KeyEvent bus should not be null");
    TEST_ASSERT_NOT_NULL_MESSAGE(&functionKeyEventBus, "FunctionKeyEvent bus should not be null");
    
    // Test event bus registration
    eventSystem.registerEventBus<KeyEvent>();
    eventSystem.registerEventBus<FunctionKeyEvent>();
    
    // Test multiple subscribers
    int keyEventCount = 0;
    int functionKeyEventCount = 0;
    
    auto keyListener1 = keyEventBus.subscribe([&](const KeyEvent& event) { keyEventCount++; });
    auto keyListener2 = keyEventBus.subscribe([&](const KeyEvent& event) { keyEventCount++; });
    auto functionKeyListener = functionKeyEventBus.subscribe([&](const FunctionKeyEvent& event) { functionKeyEventCount++; });
    
    // Publish events
    KeyEvent keyEvent;
    keyEvent.key = 'b';
    keyEvent.keyCode = 5;
    keyEvent.modifiers = 0;
    keyEvent.valid = true;
    keyEventBus.publish(keyEvent);
    
    FunctionKeyEvent functionKeyEvent(FunctionKeyEvent::Escape);
    functionKeyEventBus.publish(functionKeyEvent);
    
    // Process events
    eventSystem.processAllEvents();
    
    // Verify both listeners received the key event
    TEST_ASSERT_EQUAL_MESSAGE(2, keyEventCount, "Both key event listeners should receive the event");
    TEST_ASSERT_EQUAL_MESSAGE(1, functionKeyEventCount, "Function key event listener should receive the event");
    
    // Test clear
    keyEventBus.clear();
    functionKeyEventBus.clear();
    
    // Publish events again
    keyEventBus.publish(keyEvent);
    functionKeyEventBus.publish(functionKeyEvent);
    
    // Process events
    eventSystem.processAllEvents();
    
    // Counts should not change after clear
    TEST_ASSERT_EQUAL_MESSAGE(2, keyEventCount, "Key event count should not change after clear");
    TEST_ASSERT_EQUAL_MESSAGE(1, functionKeyEventCount, "Function key event count should not change after clear");
}
