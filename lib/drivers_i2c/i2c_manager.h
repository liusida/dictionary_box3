#pragma once

#include "Wire.h"
#include "log.h"

class I2CManager {
public:
    static I2CManager& instance();
    
    bool initialize();
    bool isReady() const;
    void shutdown();
    void setFrequency(uint32_t frequency);
    uint32_t getFrequency() const;
    
    TwoWire& getWire() { return Wire; }
    
private:
    I2CManager() = default;
    ~I2CManager() = default;
    I2CManager(const I2CManager&) = delete;
    I2CManager& operator=(const I2CManager&) = delete;
    
    bool initialized_ = false;
    uint32_t currentFrequency_ = 100000; // Default 100kHz
};


