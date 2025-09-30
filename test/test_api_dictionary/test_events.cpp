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

// Reset event tracking
void resetEventTracking() {
    lookupStartedReceived = false;
    lookupCompletedReceived = false;
    lookupFailedReceived = false;
    audioRequestedReceived = false;
}

// =================================== TESTS ===================================

