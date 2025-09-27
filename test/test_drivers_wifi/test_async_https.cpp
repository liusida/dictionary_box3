#include <Arduino.h>
#include <unity.h>
#include "log.h"
#include "memory_test_helper.h"
#include "network_control.h"
#include "test_wifi_credentials.h"
#include <WiFiClientSecure.h>

using namespace dict;

#define TAG "AsyncHTTPSTest"

#define WIFI_SSID TEST_WIFI_SSID
#define WIFI_PASSWORD TEST_WIFI_PASSWORD

void prewarm(NetworkControl& nc) {
  WiFiClientSecure client;
  client.setInsecure();
  client.connect("httpbin.org", 443);
}

void one_https_request(NetworkControl& nc) {
    // Manual async HTTPS example
    WiFiClientSecure client;
    
    // Set up SSL (use the same cert bundle as NetworkControl)
    nc.setCACertBundle(client);
    
    ESP_LOGI(TAG, "Starting manual async HTTPS request...");
    
    // Step 1: Begin connection (non-blocking)
    bool connected = client.connect("httpbin.org", 443);
    ESP_LOGI(TAG, "Connection initiated: %s", connected ? "success" : "failed");
    
    if (!connected) {
        TEST_ASSERT_TRUE_MESSAGE(false, "Failed to initiate connection");
        return;
    }
    
    // Step 2: Wait for connection to complete
    unsigned long connectStart = millis();
    while (!client.connected() && (millis() - connectStart < 5000)) {
        delay(10); // Small delay, return control to main loop
        ESP_LOGI(TAG, "Waiting for connection...");
    }
    
    if (!client.connected()) {
        ESP_LOGE(TAG, "Connection timeout");
        TEST_ASSERT_TRUE_MESSAGE(false, "Connection timeout");
        return;
    }
    
    ESP_LOGI(TAG, "Connected! Sending request...");
    
    // Step 3: Send HTTP request
    client.print("GET /json HTTP/1.1\r\n");
    client.print("Host: httpbin.org\r\n");
    client.print("User-Agent: ESP32-Test\r\n");
    client.print("Connection: close\r\n");
    client.print("\r\n");
    
    ESP_LOGI(TAG, "Request sent, waiting for response...");
    
    // Step 4: Wait for response data
    unsigned long responseStart = millis();
    while (!client.available() && (millis() - responseStart < 10000)) {
        delay(10); // Small delay, return control to main loop
    }
    
    if (!client.available()) {
        ESP_LOGE(TAG, "No response received");
        TEST_ASSERT_TRUE_MESSAGE(false, "No response received");
        client.stop();
        return;
    }
    
    ESP_LOGI(TAG, "Response received! Reading headers...");
    
    // Step 5: Read response headers
    String headers = "";
    while (client.available()) {
        String line = client.readStringUntil('\n');
        headers += line;
        
        if (line.length() <= 1) { // Empty line = end of headers
            break;
        }
        
        ESP_LOGI(TAG, "Header: %s", line.c_str());
    }
    
    ESP_LOGI(TAG, "Reading response body...");
    
    // Step 6: Read response body in chunks
    String responseBody = "";
    unsigned long bodyStart = millis();
    
    while (client.connected() || client.available()) {
        if (client.available()) {
            String chunk = client.readString();
            responseBody += chunk;
            ESP_LOGI(TAG, "Read %d bytes", chunk.length());
        } else {
            delay(10); // Small delay when no data available
        }
        
        // Safety timeout
        if (millis() - bodyStart > 15000) {
            ESP_LOGW(TAG, "Body read timeout");
            break;
        }
    }
    
    ESP_LOGI(TAG, "Response complete! Total body length: %d", responseBody.length());
    ESP_LOGI(TAG, "Response body: %s", responseBody.c_str());
    
    // Clean up
    client.stop();
    
    // Test passed if we got some response
    TEST_ASSERT_TRUE_MESSAGE(responseBody.length() > 0, "Should receive response body");
    

}

void test_async_https(void) {
    NetworkControl nc;
    nc.initialize();
    nc.randomizeMACAddress();
    nc.connectToNetwork(WIFI_SSID, WIFI_PASSWORD);

    while(!nc.isConnected()) {
        delay(100);
        nc.tick();
    } 

    ESP_LOGI(TAG, "Connected! IP: %s", nc.getIP().toString().c_str());
    nc.tick();

    prewarm(nc);

    delay(10000);

    uint32_t start = millis();
    one_https_request(nc);
    uint32_t end = millis();
    ESP_LOGI(TAG, "(1) Time taken: %dms", end - start);

    delay(1000);

    start = millis();
    one_https_request(nc);
    end = millis();
    ESP_LOGI(TAG, "(2) Time taken: %dms", end - start);

    delay(1000);

    start = millis();
    one_https_request(nc);
    end = millis();
    ESP_LOGI(TAG, "(3) Time taken: %dms", end - start);

    nc.shutdown();
}