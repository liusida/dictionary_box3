#include <Arduino.h>
#include <unity.h>
#include "log.h"
#include "memory_test_helper.h"
#include "network_control.h"
#include "test_wifi_credentials.h"

using namespace dict;

#define TAG "NetworkControlConnectTest"

#define WIFI_SSID TEST_WIFI_SSID
#define WIFI_PASSWORD TEST_WIFI_PASSWORD

void test_network_control_connect_and_report_ip(void) {
    NetworkControl nc;
    
    nc.initialize();
    nc.randomizeMACAddress();

    // Ensure a clean state
    nc.clearCredentials();

    volatile bool connectedFired = false;
    volatile bool failedFired = false;
    IPAddress connectedIp;

    nc.setOnConnected([&](const IPAddress& ip){ connectedFired = true; });
    nc.setOnConnectionFailed([&](){ failedFired = true; });

    TEST_ASSERT_TRUE_MESSAGE(nc.connectToNetwork(WIFI_SSID, WIFI_PASSWORD), "WiFi.begin should be invoked successfully");

    // Loop tick until a callback fires or timeout (15s)
    unsigned long start = millis();
    while (!connectedFired && !failedFired && millis() - start < 15000) {
        nc.tick();
        delay(100);
    }

    TEST_ASSERT_FALSE_MESSAGE(failedFired, "Connection failed callback fired");
    TEST_ASSERT_TRUE_MESSAGE(connectedFired, "Connected callback did not fire within timeout");
    if (connectedFired) {
        ESP_LOGI(TAG, "Connected. IP: %s", nc.getIP().toString().c_str());
    }

    nc.shutdown();
}

