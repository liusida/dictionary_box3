#pragma once
#include "common.h"
#include "Wire.h"
#include "core_misc/log.h"

namespace dict {

class I2CManager {
public:
    // Singleton access
    static I2CManager& instance(); // Get singleton instance of I2C manager
    
    // Core lifecycle methods
    bool initialize(); // Initialize I2C bus with default pins and frequency
    bool isReady() const; // Check if I2C bus is initialized and ready
    void shutdown(); // Clean shutdown of I2C bus
    
    // Configuration methods
    void setFrequency(uint32_t frequency); // Set I2C bus frequency (only if initialized)
    uint32_t getFrequency() const; // Get current I2C bus frequency
    
    // Utility/getter methods
    TwoWire& getWire() { return Wire; } // Get TwoWire object reference for direct access
    
private:
    I2CManager() = default;
    ~I2CManager() = default;
    I2CManager(const I2CManager&) = delete;
    I2CManager& operator=(const I2CManager&) = delete;
    
    bool initialized_ = false;
    uint32_t currentFrequency_ = 100000; // Default 100kHz
};

} // namespace dict


