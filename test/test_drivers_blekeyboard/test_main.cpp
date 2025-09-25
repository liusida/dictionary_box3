#include <Arduino.h>
#include <unity.h>
#include "memory_test_helper.h"

using namespace dict;

void test_ble_keyboard_init_and_shutdown(void);

#define TAG "BLEKeyboardTest"

void setUp(void) {
    setUpMemoryMonitoring();
}

void tearDown(void) {
    tearDownMemoryMonitoring("test");
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    printTestSuiteMemorySummary("BLEKeyboard", true);
    UNITY_BEGIN();
    RUN_TEST_EX(TAG, test_ble_keyboard_init_and_shutdown);
    UNITY_END();
    printTestSuiteMemorySummary("BLEKeyboard", false);
}

void loop() {}


