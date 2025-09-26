#include <Arduino.h>
#include <unity.h>
#include "../../lib/api_dictionary/dictionary_api.h"
#include "../../lib/core_eventing/event_system.h"
#include "../../lib/core_eventing/events.h"

using namespace dict;

// Event tracking for testing
static bool lookupStartedReceived = false;
static bool lookupCompletedReceived = false;
static bool lookupFailedReceived = false;
static bool audioRequestedReceived = false;
static DictionaryEvent lastReceivedEvent;

// Event listener for testing
void testEventListener(const DictionaryEvent& event) {
    lastReceivedEvent = event;
    switch (event.type) {
        case DictionaryEvent::LookupStarted:
            lookupStartedReceived = true;
            break;
        case DictionaryEvent::LookupCompleted:
            lookupCompletedReceived = true;
            break;
        case DictionaryEvent::LookupFailed:
            lookupFailedReceived = true;
            break;
        case DictionaryEvent::AudioRequested:
            audioRequestedReceived = true;
            break;
        default:
            break;
    }
}

// Reset event tracking
void resetEventTracking() {
    lookupStartedReceived = false;
    lookupCompletedReceived = false;
    lookupFailedReceived = false;
    audioRequestedReceived = false;
    lastReceivedEvent = DictionaryEvent();
}

// =================================== TESTS ===================================

void test_dictionary_api_event_publishing(void) {
    DictionaryApi api;
    
    // Subscribe to events
    EventSystem::instance().getEventBus<DictionaryEvent>().subscribe(testEventListener);
    resetEventTracking();
    
    api.initialize();
    
    // Test lookup with invalid word (should publish LookupFailed)
    DictionaryResult result = api.lookupWord("");
    TEST_ASSERT_FALSE(result.success);
    
    // Process events to deliver them
    EventSystem::instance().processAllEvents();
    
    // Verify events were published
    TEST_ASSERT_TRUE_MESSAGE(lookupFailedReceived, "LookupFailed event should be published");
    TEST_ASSERT_EQUAL_MESSAGE(DictionaryEvent::LookupFailed, lastReceivedEvent.type,
        ("Expected LookupFailed event, but got event type: " + String(lastReceivedEvent.type)).c_str());
    
    // Clean up
    EventSystem::instance().getEventBus<DictionaryEvent>().clear();
}

void test_dictionary_api_event_lookup_started(void) {
    DictionaryApi api;
    
    // Subscribe to events
    EventSystem::instance().getEventBus<DictionaryEvent>().subscribe(testEventListener);
    resetEventTracking();
    
    api.initialize();
    
    // Test lookup with valid word format - should publish LookupStarted with WiFi connection
    DictionaryResult result = api.lookupWord("test");
    TEST_ASSERT_TRUE(result.success); // Should succeed with WiFi connection
    
    // Process events to deliver them
    EventSystem::instance().processAllEvents();
    
    // Verify LookupStarted event was published
    TEST_ASSERT_TRUE_MESSAGE(lookupStartedReceived, "LookupStarted event should be published");
    TEST_ASSERT_EQUAL_MESSAGE(DictionaryEvent::LookupCompleted, lastReceivedEvent.type, 
        ("Expected LookupCompleted event, but got event type: " + String(lastReceivedEvent.type)).c_str());
    TEST_ASSERT_EQUAL_STRING("test", lastReceivedEvent.word.c_str());
    
    // Clean up
    EventSystem::instance().getEventBus<DictionaryEvent>().clear();
}

void test_dictionary_api_event_audio_requested(void) {
    DictionaryApi api;
    
    // Subscribe to events
    EventSystem::instance().getEventBus<DictionaryEvent>().subscribe(testEventListener);
    resetEventTracking();
    
    api.initialize();
    
    // Test audio URL generation - should publish AudioRequested with WiFi connection
    AudioUrl audioUrl = api.getAudioUrl("test", "word");
    TEST_ASSERT_TRUE(audioUrl.valid); // Should succeed with WiFi connection
    
    // Process events to deliver them
    EventSystem::instance().processAllEvents();
    
    // Verify AudioRequested event was published
    TEST_ASSERT_TRUE_MESSAGE(audioRequestedReceived, "AudioRequested event should be published");
    TEST_ASSERT_EQUAL_MESSAGE(DictionaryEvent::AudioRequested, lastReceivedEvent.type,
        ("Expected AudioRequested event, but got event type: " + String(lastReceivedEvent.type)).c_str());
    TEST_ASSERT_EQUAL_STRING("test", lastReceivedEvent.word.c_str());
    TEST_ASSERT_EQUAL_STRING("word", lastReceivedEvent.errorMessage.c_str());
    
    // Clean up
    EventSystem::instance().getEventBus<DictionaryEvent>().clear();
}

void test_dictionary_api_event_multiple_subscribers(void) {
    DictionaryApi api;
    
    // Subscribe multiple listeners
    int listener1Count = 0;
    int listener2Count = 0;
    
    auto listener1 = [&](const DictionaryEvent& event) {
        if (event.type == DictionaryEvent::LookupStarted) {
            listener1Count++;
        }
    };
    
    auto listener2 = [&](const DictionaryEvent& event) {
        if (event.type == DictionaryEvent::LookupStarted) {
            listener2Count++;
        }
    };
    
    EventSystem::instance().getEventBus<DictionaryEvent>().subscribe(listener1);
    EventSystem::instance().getEventBus<DictionaryEvent>().subscribe(listener2);
    
    api.initialize();
    
    // Test lookup
    DictionaryResult result = api.lookupWord("test");
    
    // Process events to deliver them
    EventSystem::instance().processAllEvents();
    
    // Verify both listeners were called
    TEST_ASSERT_EQUAL(1, listener1Count);
    TEST_ASSERT_EQUAL(1, listener2Count);
    
    // Clean up
    EventSystem::instance().getEventBus<DictionaryEvent>().clear();
}

void test_dictionary_api_event_cleanup(void) {
    DictionaryApi api;
    
    // Subscribe to events
    EventSystem::instance().getEventBus<DictionaryEvent>().subscribe(testEventListener);
    resetEventTracking();
    
    api.initialize();
    
    // Test lookup
    DictionaryResult result = api.lookupWord("test");
    
    // Process events
    EventSystem::instance().processAllEvents();
    
    // Verify event was received
    TEST_ASSERT_TRUE(lookupStartedReceived);
    
    // Clear event bus
    EventSystem::instance().getEventBus<DictionaryEvent>().clear();
    resetEventTracking();
    
    // Test another lookup
    DictionaryResult result2 = api.lookupWord("test2");
    
    // Process events
    EventSystem::instance().processAllEvents();
    
    // Verify no events were received (bus was cleared)
    TEST_ASSERT_FALSE(lookupStartedReceived);
    TEST_ASSERT_FALSE(lookupCompletedReceived);
    TEST_ASSERT_FALSE(lookupFailedReceived);
    TEST_ASSERT_FALSE(audioRequestedReceived);
}
