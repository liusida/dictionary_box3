#pragma once
#include "core/event_system.h"
#include "core/events.h"
#include <functional>

/**
 * @brief Connection health monitoring and failure detection
 * 
 * Continuously monitors connectivity services and detects failures/recovery.
 * Publishes events that the UI can respond to for automatic recovery flow.
 */
class ConnectionMonitor {
public:
    static ConnectionMonitor& instance();
    
    // Lifecycle
    void initialize();
    void shutdown();
    void tick(); // Call in main loop
    
    // Service status
    bool isWiFiHealthy() const;
    bool isBLEHealthy() const;
    bool isAnyConnectivityHealthy() const;
    
    // Configuration
    void setFailureThreshold(unsigned long thresholdMs);
    void setRecoveryAttempts(int maxAttempts);
    
    // Manual triggers
    void triggerWiFiRecovery();
    void triggerBLERecovery();
    
private:
    ConnectionMonitor() = default;
    ~ConnectionMonitor() = default;
    
    // Disable copy constructor and assignment operator
    ConnectionMonitor(const ConnectionMonitor&) = delete;
    ConnectionMonitor& operator=(const ConnectionMonitor&) = delete;
    
    // Monitoring state
    bool wifiHealthy_ = false;
    bool bleHealthy_ = false;
    unsigned long wifiLastSeen_ = 0;
    unsigned long bleLastSeen_ = 0;
    unsigned long failureThreshold_ = 10000; // 10 seconds
    int wifiRecoveryAttempts_ = 0;
    int bleRecoveryAttempts_ = 0;
    int maxRecoveryAttempts_ = 3;
    
    // State tracking
    bool initialized_ = false;
    unsigned long lastCheckTime_ = 0;
    static const unsigned long CHECK_INTERVAL_MS = 2000; // Check every 2 seconds
    
    // Internal methods
    void checkWiFiHealth();
    void checkBLEHealth();
    void publishServiceStatusEvent(const String& service, bool healthy);
    void attemptWiFiRecovery();
    void attemptBLERecovery();
};
