#pragma once

// Touch pin definitions for ESP32-S3-Box-3
#define TOUCH_INT_PIN 3 // BSP_LCD_TOUCH_INT
#define TOUCH_RESET_PIN 48
// use SHARED_I2C_SDA and SHARED_I2C_SCL from pins_arduino.h
// #define TOUCH_I2C_SDA 8   // BSP_I2C_SDA
// #define TOUCH_I2C_SCL 18  // BSP_I2C_SCL

// Touch configuration
#define TOUCH_I2C_ADDR 0x5D   // GT911 default address
// #define TOUCH_I2C_FREQ 100000 // 100kHz for compatibility