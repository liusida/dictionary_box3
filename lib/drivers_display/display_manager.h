#pragma once
#include "TFT_eSPI.h"
#include "GT911.h"
#include "lvgl.h"
#include "i2c_manager.h"

class DisplayManager {
public:
    DisplayManager();
    ~DisplayManager();
    
    bool initialize();
    void shutdown();
    void tick();
    bool isReady() const;
    
    void resetDisplay();
    void setBacklight(bool on);
    
    bool initTouch();
    void handleTouch();
    
    void initLVGL();
    void handleLVGLTasks();
    
    TFT_eSPI& getTFT() { return tft_; }
    GT911& getTouch() { return touch_; }
    
    static void dispFlushCallback(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map);
    static void touchpadReadCallback(lv_indev_t *indev_driver, lv_indev_data_t *data);
    static uint32_t tickCallback();
    
private:
    TFT_eSPI tft_;
    GT911 touch_;
    bool displayInitialized_;
    bool touchInitialized_;
    bool lvglInitialized_;
    
    // LVGL objects and buffers
    lv_display_t *display_;
    lv_indev_t *inputDevice_;
    lv_color_t *buffer1_;
    lv_color_t *buffer2_;
};


