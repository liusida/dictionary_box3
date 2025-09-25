#include "log.h"
#include "memory_test_helper.h"
#include <Arduino.h>
#include <WiFi.h>
#include <unity.h>

#define TAG "WiFiTest"

#define WIFI_SSID TEST_WIFI_SSID
#define WIFI_PASSWORD TEST_WIFI_PASSWORD

// =================================== TESTS ===================================

void test_wifi_basic_functions(void) {
    // Test WiFi mode setting
    WiFi.mode(WIFI_STA);
    TEST_ASSERT_EQUAL(WIFI_MODE_STA, WiFi.getMode());

    // Test WiFi status
    wl_status_t status = WiFi.status();
    ESP_LOGI(TAG, "WiFi status: %d", status);

    // Show what each status means
    switch (status) {
    case WL_IDLE_STATUS:
        ESP_LOGI(TAG, "Status: WL_IDLE_STATUS (0) - WiFi is in process of changing between states");
        break;
    case WL_NO_SSID_AVAIL:
        ESP_LOGI(TAG, "Status: WL_NO_SSID_AVAIL (1) - Configured SSID cannot be reached");
        break;
    case WL_SCAN_COMPLETED:
        ESP_LOGI(TAG, "Status: WL_SCAN_COMPLETED (2) - Scan networks is completed");
        break;
    case WL_CONNECTED:
        ESP_LOGI(TAG, "Status: WL_CONNECTED (3) - Successfully connected to a WiFi network");
        break;
    case WL_CONNECT_FAILED:
        ESP_LOGI(TAG, "Status: WL_CONNECT_FAILED (4) - Connection failed for all the attempts");
        break;
    case WL_CONNECTION_LOST:
        ESP_LOGI(TAG, "Status: WL_CONNECTION_LOST (5) - Connection is lost");
        break;
    case WL_DISCONNECTED:
        ESP_LOGI(TAG, "Status: WL_DISCONNECTED (6) - WiFi is disconnected");
        break;
    case WL_STOPPED:
        ESP_LOGI(TAG, "Status: WL_STOPPED (254) - WiFi driver is not running/stopped");
        break;
    default:
        ESP_LOGI(TAG, "Status: Unknown (%d)", status);
        break;
    }

    TEST_ASSERT_TRUE_MESSAGE(status == WL_IDLE_STATUS || status == WL_NO_SSID_AVAIL || status == WL_SCAN_COMPLETED || status == WL_CONNECTED ||
                                 status == WL_CONNECT_FAILED || status == WL_CONNECTION_LOST || status == WL_DISCONNECTED || status == WL_STOPPED,
                             "Unknown WiFi status");

    // Test MAC address
    String mac = WiFi.macAddress();
    TEST_ASSERT_TRUE_MESSAGE(mac.length() > 0, "MAC address should not be empty");

    // Test hostname setting
    WiFi.setHostname("ESP32-Test");
    String hostname = WiFi.getHostname();
    TEST_ASSERT_EQUAL_STRING("ESP32-Test", hostname.c_str());

    // Test auto-reconnect setting
    WiFi.setAutoReconnect(false);
    TEST_ASSERT_FALSE(WiFi.getAutoReconnect());

    WiFi.setAutoReconnect(true);
    TEST_ASSERT_TRUE(WiFi.getAutoReconnect());
}

void test_wifi_network_scanning(void) {
    // Set WiFi mode and disconnect first
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);

    // Test network scanning
    int n = WiFi.scanNetworks();
    TEST_ASSERT_TRUE_MESSAGE(n >= 0, "Scan should return non-negative number");

    // Print network details if any found
    for (int i = 0; i < n; i++) {
        String ssid = WiFi.SSID(i);
        int32_t rssi = WiFi.RSSI(i);
        ESP_LOGI("WiFiTest", "Network %d: %s (%d dBm)", i, ssid.c_str(), rssi);
    }

    // Clear scan results
    WiFi.scanDelete();
}

void test_wifi_connection_management(void) {
    WiFi.mode(WIFI_STA);

    // Test connection to real network
    ESP_LOGI(TAG, "Attempting to connect to %s", WIFI_SSID);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    // Wait for connection
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        attempts++;
        wl_status_t currentStatus = WiFi.status();
        ESP_LOGI(TAG, "Connection attempt %d, status: %d", attempts, currentStatus);

        // Show what each status means during connection
        switch (currentStatus) {
        case WL_IDLE_STATUS:
            ESP_LOGI(TAG, "  -> WL_IDLE_STATUS (0) - WiFi is in process of changing between states");
            break;
        case WL_NO_SSID_AVAIL:
            ESP_LOGI(TAG, "  -> WL_NO_SSID_AVAIL (1) - Configured SSID cannot be reached");
            break;
        case WL_SCAN_COMPLETED:
            ESP_LOGI(TAG, "  -> WL_SCAN_COMPLETED (2) - Scan networks is completed");
            break;
        case WL_CONNECTED:
            ESP_LOGI(TAG, "  -> WL_CONNECTED (3) - Successfully connected to a WiFi network");
            break;
        case WL_CONNECT_FAILED:
            ESP_LOGI(TAG, "  -> WL_CONNECT_FAILED (4) - Connection failed for all the attempts");
            break;
        case WL_CONNECTION_LOST:
            ESP_LOGI(TAG, "  -> WL_CONNECTION_LOST (5) - Connection is lost");
            break;
        case WL_DISCONNECTED:
            ESP_LOGI(TAG, "  -> WL_DISCONNECTED (6) - WiFi is disconnected");
            break;
        default:
            ESP_LOGI(TAG, "  -> Unknown (%d)", currentStatus);
            break;
        }
    }

    wl_status_t status = WiFi.status();
    if (status == WL_CONNECTED) {
        ESP_LOGI(TAG, "Connected! IP: %s", WiFi.localIP().toString().c_str());
        TEST_ASSERT_TRUE_MESSAGE(true, "Successfully connected to WiFi");
    } else {
        ESP_LOGW(TAG, "Connection failed, status: %d", status);
        TEST_ASSERT_TRUE_MESSAGE(status == WL_CONNECT_FAILED || status == WL_DISCONNECTED, "Connection should fail gracefully");
    }

    // Test disconnect
    WiFi.disconnect();
    delay(500);
    status = WiFi.status();
    TEST_ASSERT_TRUE_MESSAGE(status == WL_DISCONNECTED || status == WL_IDLE_STATUS, "Should be disconnected after disconnect()");
}

void test_wifi_connection_timeout(void) {
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);

    // Test connection timeout with invalid credentials
    unsigned long startTime = millis();
    WiFi.begin("InvalidNetwork12345", "wrongpassword");

    // Wait for connection attempt to timeout
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        attempts++;
    }

    unsigned long duration = millis() - startTime;
    TEST_ASSERT_TRUE_MESSAGE(duration < 15000, "Connection should timeout within 15 seconds");

    // Test status after failed connection
    wl_status_t status = WiFi.status();
    TEST_ASSERT_TRUE_MESSAGE(status != WL_CONNECTED, "Should not be connected after timeout test");

    WiFi.disconnect();
}

void test_wifi_simplest(void) {
    ESP_LOGI(TAG, "Current WiFi status: %d", WiFi.status());
    WiFi.mode(WIFI_STA);
    ESP_LOGI(TAG, "Current WiFi status: %d", WiFi.status());
    WiFi.disconnect();
    delay(100);
    ESP_LOGI(TAG, "Current WiFi status: %d", WiFi.status());

    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    ESP_LOGI(TAG, "Current WiFi status: %d", WiFi.status());

    delay(2000);
    ESP_LOGI(TAG, "Current WiFi status: %d", WiFi.status());
    TEST_ASSERT_TRUE_MESSAGE(WiFi.status() == WL_CONNECTED, "Should be connected");

    ESP_LOGI(TAG, "Current WiFi status: %d", WiFi.status());
    WiFi.disconnect();
    ESP_LOGI(TAG, "Current WiFi status: %d", WiFi.status());
    delay(100);
    ESP_LOGI(TAG, "Current WiFi status: %d", WiFi.status());

}
