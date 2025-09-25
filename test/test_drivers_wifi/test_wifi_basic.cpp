#include <Arduino.h>
#include <WiFi.h>
#include <unity.h>
#include "log.h"
#include "memory_test_helper.h"
#include "test_wifi_credentials.h"

#define TAG "WiFiTest"

#define WIFI_SSID TEST_WIFI_SSID
#define WIFI_PASSWORD TEST_WIFI_PASSWORD

// =================================== TESTS ===================================

void test_wifi_simplest(void) {
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);

    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    delay(2000);
    TEST_ASSERT_TRUE_MESSAGE(WiFi.status() == WL_CONNECTED, "Should be connected");

    WiFi.disconnect();
    delay(100);
}
