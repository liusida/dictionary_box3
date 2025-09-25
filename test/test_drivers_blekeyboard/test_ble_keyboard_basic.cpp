#include <Arduino.h>
#include <unity.h>
#include "ble_keyboard.h"
#include "key_processor.h"
#include "lvgl_helper.h"
#include "event_system.h"

#define TAG "BLEKeyboardTest"

void test_ble_keyboard_init_and_shutdown(void) {
    BLEKeyboard kb;
    KeyProcessor kp;
    TEST_ASSERT_TRUE_MESSAGE(kb.initialize(), "BLE keyboard initialize should return true");
    // Allow some time for NimBLE init and scanning start
    unsigned long start = millis();
    while (!kb.isConnected() && millis() - start < 15000) {
        kb.tick();
        delay(100);
    }

    TEST_ASSERT_TRUE_MESSAGE(kb.isConnected(), "BLE keyboard should be connected");

    kb.setKeyCallback([&](char ch, uint8_t keyCode, uint8_t modifiers){
        kp.sendKeyToLVGL(ch, keyCode, modifiers);
    });

    // let's process all the queued keys and events
    unsigned long processStart = millis();
    while (millis() - processStart < 2000) { // process for 2s
        kp.tick();
        core::eventing::EventSystem::instance().processAllEvents();
        delay(50);
    }

    kb.shutdown();
    // Basic smoke: no crash and returns
    TEST_ASSERT_TRUE(true);
}


