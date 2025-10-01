#include <Arduino.h>
#include <unity.h>
#include "../../lib/drivers_network/network_control.h"
#include "../../lib/core_misc/memory_test_helper.h"

using namespace dict;

// =================================== TESTS ===================================

void test_network_control_initialize_and_ready(void) {
    NetworkControl& network = NetworkControl::instance();
    TEST_ASSERT_NOT_NULL_MESSAGE(&network, "NetworkControl singleton should not be null");
    
    // Test initialization
    TEST_ASSERT_TRUE_MESSAGE(network.initialize(), "NetworkControl initialize() failed");
    TEST_ASSERT_TRUE_MESSAGE(network.isReady(), "NetworkControl should be ready after initialization");
    
    // Test that we can call isReady multiple times
    TEST_ASSERT_TRUE(network.isReady());
    TEST_ASSERT_TRUE(network.isReady());
    
    // Test shutdown
    network.shutdown();
    TEST_ASSERT_FALSE_MESSAGE(network.isReady(), "NetworkControl should not be ready after shutdown");
}

void test_network_control_tick_safety(void) {
    NetworkControl& network = NetworkControl::instance();
    TEST_ASSERT_TRUE_MESSAGE(network.initialize(), "NetworkControl initialize() failed");
    TEST_ASSERT_TRUE(network.isReady());
    
    // Test that tick() can be called safely when ready
    network.tick();
    network.tick();
    network.tick();
    
    // Verify network is still ready after ticks
    TEST_ASSERT_TRUE(network.isReady());
    
    // Test tick() safety when not ready
    network.shutdown();
    network.tick(); // Should not crash
    network.tick(); // Should not crash
    
    TEST_ASSERT_FALSE(network.isReady());
}

void test_network_control_connection_state(void) {
    NetworkControl& network = NetworkControl::instance();
    TEST_ASSERT_TRUE_MESSAGE(network.initialize(), "NetworkControl initialize() failed");
    TEST_ASSERT_TRUE(network.isReady());
    
    // Test connection state methods
    TEST_ASSERT_FALSE_MESSAGE(network.isConnected(), "Should not be connected initially");
    TEST_ASSERT_FALSE_MESSAGE(network.isConnecting(), "Should not be connecting initially");
    
    // Test connection timing methods
    uint32_t connectStartTime = network.getConnectStartTime();
    uint32_t connectEndTime = network.getConnectEndTime();
    
    // Times should be valid
    TEST_ASSERT_TRUE_MESSAGE(connectStartTime >= 0, "Connect start time should be valid");
    TEST_ASSERT_TRUE_MESSAGE(connectEndTime >= 0, "Connect end time should be valid");
    
    // Test status methods
    wl_status_t status = network.getStatus();
    TEST_ASSERT_TRUE_MESSAGE(status >= WL_NO_SSID_AVAIL, "WiFi status should be valid");
    
    // Test IP address (should be 0.0.0.0 when not connected)
    IPAddress ip = network.getIP();
    TEST_ASSERT_EQUAL_MESSAGE(0, ip[0], "IP should be 0.0.0.0 when not connected");
    
    // Test MAC address randomization (should not crash)
    network.randomizeMACAddress();
    
    // Test disconnect (should not crash)
    network.disconnect();
    
    // Verify still ready after operations
    TEST_ASSERT_TRUE(network.isReady());
    
    network.shutdown();
}

void test_network_control_credential_management(void) {
    NetworkControl& network = NetworkControl::instance();
    TEST_ASSERT_TRUE_MESSAGE(network.initialize(), "NetworkControl initialize() failed");
    TEST_ASSERT_TRUE(network.isReady());
    
    // Test current credentials (should be empty)
    String currentSsid = network.getCurrentSsid();
    String currentPassword = network.getCurrentPassword();
    TEST_ASSERT_TRUE_MESSAGE(currentSsid.isEmpty(), "Current SSID should be empty initially");
    TEST_ASSERT_TRUE_MESSAGE(currentPassword.isEmpty(), "Current password should be empty initially");
    
    // Test credential operations
    network.saveCredentials("test_ssid", "test_password");
    TEST_ASSERT_TRUE_MESSAGE(network.hasSavedCredentials(), "Should have saved credentials after save");
    
    String loadedSsid, loadedPassword;
    TEST_ASSERT_TRUE_MESSAGE(network.loadCredentials(loadedSsid, loadedPassword), "Should load credentials successfully");
    TEST_ASSERT_EQUAL_STRING_MESSAGE("test_ssid", loadedSsid.c_str(), "Loaded SSID should match");
    TEST_ASSERT_EQUAL_STRING_MESSAGE("test_password", loadedPassword.c_str(), "Loaded password should match");
    
    // Test clear credentials
    network.clearCredentials();
    TEST_ASSERT_FALSE_MESSAGE(network.hasSavedCredentials(), "Should not have saved credentials after clear");
    
    // Test set trying credentials
    network.setTryingCredentials("trying_ssid", "trying_password");
    
    // Verify still ready after credential operations
    TEST_ASSERT_TRUE(network.isReady());
    
    network.shutdown();
}

void test_network_control_scanning_functionality(void) {
    NetworkControl& network = NetworkControl::instance();
    TEST_ASSERT_TRUE_MESSAGE(network.initialize(), "NetworkControl initialize() failed");
    TEST_ASSERT_TRUE(network.isReady());
    
    // Test scanning state
    TEST_ASSERT_FALSE_MESSAGE(network.isScanning(), "Should not be scanning initially");
    
    // Test scan networks (may return empty list without WiFi hardware)
    std::vector<String> networks = network.scanNetworks();
    TEST_ASSERT_NOT_NULL_MESSAGE(&networks, "Scan networks should return valid vector");
    
    // Test scanning state control
    network.setScanning(true);
    TEST_ASSERT_TRUE_MESSAGE(network.isScanning(), "Should be scanning after setScanning(true)");
    
    network.setScanning(false);
    TEST_ASSERT_FALSE_MESSAGE(network.isScanning(), "Should not be scanning after setScanning(false)");
    
    // Test settings screen state
    TEST_ASSERT_FALSE_MESSAGE(network.isOnSettingScreen(), "Should not be on setting screen initially");
    
    network.setIsOnSettingScreen(true);
    TEST_ASSERT_TRUE_MESSAGE(network.isOnSettingScreen(), "Should be on setting screen after setIsOnSettingScreen(true)");
    
    network.setIsOnSettingScreen(false);
    TEST_ASSERT_FALSE_MESSAGE(network.isOnSettingScreen(), "Should not be on setting screen after setIsOnSettingScreen(false)");
    
    // Verify still ready after scanning operations
    TEST_ASSERT_TRUE(network.isReady());
    
    network.shutdown();
}
