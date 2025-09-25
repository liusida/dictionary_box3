#include "i2c_manager.h"
#include "pins_arduino.h"

namespace dict {

static const char* TAG = "I2CManager";

I2CManager& I2CManager::instance() {
    static I2CManager instance;
    return instance;
}

bool I2CManager::initialize() {
    if (initialized_) {
        ESP_LOGI(TAG, "I2C already initialized");
        return true;
    }
    
    ESP_LOGI(TAG, "Initializing shared I2C bus...");
    ESP_LOGI(TAG, "I2C pins: SDA=%d, SCL=%d", SHARED_I2C_SDA, SHARED_I2C_SCL);
    
    if (!Wire.begin(SHARED_I2C_SDA, SHARED_I2C_SCL)) {
        ESP_LOGE(TAG, "Failed to initialize I2C bus");
        return false;
    }
    
    Wire.setClock(currentFrequency_);
    
    initialized_ = true;
    ESP_LOGI(TAG, "I2C initialized successfully at %d Hz", currentFrequency_);
    ESP_LOGI(TAG, "Bus Number: %d", Wire.getBusNum());
    return true;
}

bool I2CManager::isReady() const {
    return initialized_;
}

void I2CManager::setFrequency(uint32_t frequency) {
    if (initialized_) {
        Wire.setClock(frequency);
        currentFrequency_ = frequency;
        ESP_LOGI(TAG, "I2C frequency changed to %d Hz", frequency);
    } else {
        ESP_LOGW(TAG, "Cannot set frequency - I2C not initialized");
    }
}

uint32_t I2CManager::getFrequency() const {
    return currentFrequency_;
}

void I2CManager::shutdown() {
    if (initialized_) {
        Wire.end();
        initialized_ = false;
        ESP_LOGI(TAG, "I2C shutdown");
    }
}

} // namespace dict
