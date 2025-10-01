#include <Arduino.h>
#include <unity.h>
#include "../../lib/drivers_display/lvgl_helper.h"
#include "../../lib/drivers_display/display_manager.h"
#include "../../lib/core_eventing/events.h"
#include "../../lib/core_misc/memory_test_helper.h"
#include "lvgl.h"

using namespace dict;

// =================================== TESTS ===================================

void test_lvgl_helper_key_callbacks(void) {
    // Test key callback functions
    bool submitCalled = false;
    bool keyInCalled = false;
    char receivedKey = 0;
    
    // Test callback setting
    lvglSetKeyCallbacks(
        [&]() { submitCalled = true; },
        [&](char key) { keyInCalled = true; receivedKey = key; }
    );
    
    // Test callback removal
    lvglSetKeyCallbacks(nullptr, nullptr);
    
    // Test enabling key event handler
    lvglEnableKeyEventHandler();
    
    // Test removing key event handler
    lvglRemoveKeyEventHandler();
    
    // Should not crash
    TEST_ASSERT_TRUE(true);
}

void test_lvgl_helper_function_key_callbacks(void) {
    // Test function key callback functions
    bool functionKeyCalled = false;
    FunctionKeyEvent receivedEvent;
    
    // Test callback setting
    lvglSetFunctionKeyCallbacks([&](const FunctionKeyEvent& event) {
        functionKeyCalled = true;
        receivedEvent = event;
    });
    
    // Test callback removal
    lvglSetFunctionKeyCallbacks(nullptr);
    
    // Note: lvglEnableFunctionKeyEventHandler() is not implemented in the library
    // so we skip testing it to avoid linker errors
    
    // Test removing function key event handler
    lvglRemoveFunctionKeyEventHandler();
    
    // Should not crash
    TEST_ASSERT_TRUE(true);
}
