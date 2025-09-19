#pragma once
#include "core/driver_interface.h"
#include "display.h"
#include "TFT_eSPI.h"
#include "GT911.h"

/**
 * @brief Display manager that wraps display and touch functionality
 * 
 * Provides a clean interface for display operations and implements
 * the DriverInterface for consistent lifecycle management.
 */
class DisplayManager : public DriverInterface {
public:
    DisplayManager();
    ~DisplayManager();
    
    // DriverInterface implementation
    bool initialize() override;
    void shutdown() override;
    void tick() override;
    bool isReady() const override;
    
    // Display operations
    void resetDisplay();
    void setBacklight(bool on);
    
    // Touch operations
    bool initTouch();
    void handleTouch();
    
    // LVGL operations
    void initLVGL();
    void handleLVGLTasks();
    
    // Access to underlying objects (for compatibility)
    TFT_eSPI& getTFT() { return tft_; }
    GT911& getTouch() { return touch_; }
    
private:
    TFT_eSPI tft_;
    GT911 touch_;
    bool displayInitialized_;
    bool touchInitialized_;
    bool lvglInitialized_;
};
