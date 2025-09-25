#include <Arduino.h>
#include <unity.h>
#include "i2c_manager.h"
#include "memory_test_helper.h"
#define TAG "I2CTest"

// What are tested here:
// Singleton identity: instance() returns the same object across calls.
void test_singleton_identity(void);
// Default before init: getFrequency() is 100000 Hz before initialize().
void test_not_ready_before_init_and_default_freq(void);
// setFrequency pre-init: calling setFrequency() before initialize() has no effect.
void test_set_freq_before_init_does_not_change_value(void);
// initialize readiness: initialize() succeeds and sets isReady() to true.
void test_initialize_and_ready(void);
// setFrequency post-init: setFrequency() after initialize() updates getFrequency() to 400000 Hz.
void test_set_freq_after_init_changes_value(void);
// initialize idempotent: calling initialize() multiple times keeps success and ready state.
void test_initialize_idempotent(void);
// AudioTools I2C issue: Wire object appears initialized but transactions fail
void test_audiotools_i2c_issue(void);

// =================================== TESTS ===================================
void test_singleton_identity(void) {
    I2CManager &a = I2CManager::instance();
    I2CManager &b = I2CManager::instance();
    TEST_ASSERT_EQUAL_PTR(&a, &b);
}

// Before init, default frequency is 100kHz and not ready.
void test_not_ready_before_init_and_default_freq(void) {
    I2CManager &mgr = I2CManager::instance();
    // We cannot reset the singleton in this simple test harness; assume fresh boot per test run
    // Check default frequency API value
    TEST_ASSERT_EQUAL_UINT32(100000, mgr.getFrequency());
}

// setFrequency() before init should not take effect.
void test_set_freq_before_init_does_not_change_value(void) {
    I2CManager &mgr = I2CManager::instance();
    // Attempt to change before init: should not take effect per implementation
    mgr.setFrequency(400000);
    TEST_ASSERT_EQUAL_UINT32(100000, mgr.getFrequency());
}

// initialize() should succeed and set ready flag.
void test_initialize_and_ready(void) {
    I2CManager &mgr = I2CManager::instance();
    bool ok = mgr.initialize();
    TEST_ASSERT_TRUE_MESSAGE(ok, "I2C initialize() failed");
    TEST_ASSERT_TRUE(mgr.isReady());
}

// After init, setFrequency() updates the clock to the requested value.
void test_set_freq_after_init_changes_value(void) {
    I2CManager &mgr = I2CManager::instance();
    // Ensure initialized
    if (!mgr.isReady()) {
        (void)mgr.initialize();
    }
    mgr.setFrequency(400000);
    TEST_ASSERT_EQUAL_UINT32(400000, mgr.getFrequency());
}

// Multiple initialize() calls should be idempotent.
void test_initialize_idempotent(void) {
    I2CManager &mgr = I2CManager::instance();
    bool ok1 = mgr.initialize();
    bool ok2 = mgr.initialize();
    TEST_ASSERT_TRUE(ok1);
    TEST_ASSERT_TRUE(ok2);
    TEST_ASSERT_TRUE(mgr.isReady());
}

// AudioTools I2C issue: Reproduces the scenario where Wire object appears initialized
// but I2C transactions fail with "bus is not initialized" error
void test_audiotools_i2c_issue(void) {
    I2CManager &mgr = I2CManager::instance();
    
    // 1. Initialize I2C through I2CManager (simulates your code)
    bool init_ok = mgr.initialize();
    TEST_ASSERT_TRUE_MESSAGE(init_ok, "I2CManager initialization failed");
    TEST_ASSERT_TRUE(mgr.isReady());
    
    // 2. Get the Wire object (simulates AudioTools getting the Wire reference)
    TwoWire &wire = mgr.getWire();
    
    // 3. Check that Wire object appears to be initialized
    // (This is what AudioTools sees - the object exists and has metadata)
    TEST_ASSERT_NOT_NULL(&wire);
    
    // 4. Attempt I2C transaction (simulates ES8311 driver trying to write)
    // This should fail with "bus is not initialized" error
    wire.beginTransmission(0x18); // ES8311 address
    wire.write(0x00); // Register address
    wire.write(0x00); // Data
    uint8_t result = wire.endTransmission();
    
    // The transaction should succeed if the I2C bus is properly initialized
    // Currently it fails with "bus is not initialized" error, which is the bug
    TEST_ASSERT_EQUAL_MESSAGE(0, result, 
        "I2C transaction failed - this indicates the AudioTools I2C issue exists");
    
    // Log the result for debugging
    Serial.printf("I2C transaction result: %d (0=success, 1=data too long, 2=NACK, 3=timeout, 4=other)\n", result);
}

void setUp(void) {}
void tearDown(void) {}

void setup() {
    Serial.begin(115200);
    delay(1000);
    UNITY_BEGIN();
    RUN_TEST_EX(TAG, test_singleton_identity);
    RUN_TEST_EX(TAG, test_not_ready_before_init_and_default_freq);
    RUN_TEST_EX(TAG, test_set_freq_before_init_does_not_change_value);
    RUN_TEST_EX(TAG, test_initialize_and_ready);
    RUN_TEST_EX(TAG, test_set_freq_after_init_changes_value);
    RUN_TEST_EX(TAG, test_initialize_idempotent);
    RUN_TEST_EX(TAG, test_audiotools_i2c_issue);
    UNITY_END();
}

void loop() {}


