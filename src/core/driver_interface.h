#pragma once

/**
 * @brief Base interface for all system drivers
 * 
 * Provides a consistent interface for driver lifecycle management
 * and status checking across all system components.
 */
class DriverInterface {
public:
    virtual ~DriverInterface() = default;
    
    /**
     * @brief Initialize the driver
     * @return true if initialization successful, false otherwise
     */
    virtual bool initialize() = 0;
    
    /**
     * @brief Shutdown the driver and clean up resources
     */
    virtual void shutdown() = 0;
    
    /**
     * @brief Periodic tick function - call this in main loop
     */
    virtual void tick() = 0;
    
    /**
     * @brief Check if driver is ready for use
     * @return true if ready, false otherwise
     */
    virtual bool isReady() const = 0;
};
