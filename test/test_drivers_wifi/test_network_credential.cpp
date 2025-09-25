#include <Arduino.h>
#include <unity.h>
#include "log.h"
#include "memory_test_helper.h"
#include "network_control.h"

#define TAG "NetworkControlTest"

static void clearAll(NetworkControl &nc) {
    nc.clearCredentials();
}

void test_save_and_has_saved_credentials(void) {
    NetworkControl nc;
    clearAll(nc);
    TEST_ASSERT_FALSE_MESSAGE(nc.hasSavedCredentials(), "Expect no saved credentials initially");

    nc.saveCredentials("TestSSID", "TestPWD");
    TEST_ASSERT_TRUE_MESSAGE(nc.hasSavedCredentials(), "Expect saved credentials to be detected");
}

void test_load_credentials(void) {
    NetworkControl nc;
    clearAll(nc);

    String ssid, pwd;
    TEST_ASSERT_FALSE_MESSAGE(nc.loadCredentials(ssid, pwd), "Load should fail when none saved");
    TEST_ASSERT_TRUE_MESSAGE(ssid.length() == 0 && pwd.length() == 0, "SSID/PWD should be empty");

    nc.saveCredentials("SSID2", "PWD2");
    TEST_ASSERT_TRUE_MESSAGE(nc.loadCredentials(ssid, pwd), "Load should succeed after save");
    TEST_ASSERT_EQUAL_STRING_MESSAGE("SSID2", ssid.c_str(), "SSID mismatch");
    TEST_ASSERT_EQUAL_STRING_MESSAGE("PWD2", pwd.c_str(), "PWD mismatch");
}

void test_clear_credentials(void) {
    NetworkControl nc;
    nc.saveCredentials("SSID3", "PWD3");
    TEST_ASSERT_TRUE_MESSAGE(nc.hasSavedCredentials(), "Expect saved credentials before clear");
    nc.clearCredentials();
    TEST_ASSERT_FALSE_MESSAGE(nc.hasSavedCredentials(), "Expect no saved credentials after clear");
}