#include "display_manager.h"
#include "ui_status.h"
#include <Arduino.h>
#include <unity.h>

using namespace dict;

// External reference to global display from test_main.cpp
extern DisplayManager *g_display;

// =================================== TESTS ===================================
void test_minimal_direct_create(void) {
    lv_obj_t *scr = lv_obj_create(NULL);
    lv_screen_load(scr);

    // create an object without parent
    lv_obj_t *my = lv_obj_create(scr);
    lv_obj_set_size(my, 50, 30);
    lv_obj_set_style_bg_color(my, lv_color_hex(0xFF0000), 0);
    lv_obj_set_style_bg_opa(my, LV_OPA_COVER, 0);

    // now set parent to the screen
    // lv_obj_set_parent(my, scr);

    // also set a position within screen
    lv_obj_set_pos(my, 10, 10);
    for (int i = 0; i < 1000; i++) {
        g_display->tick();
        delay(10);
    }
    lv_obj_del(my);
    lv_obj_del(scr);
}

void test_minimal_create_attach(void) {

    lv_obj_t *scr = lv_obj_create(NULL);
    lv_screen_load(scr);

    // create an object without parent
    lv_obj_t *tmp = lv_obj_create(NULL); // just for attach to work
    lv_obj_t *my = lv_obj_create(tmp);
    lv_obj_set_size(my, 50, 30);
    lv_obj_set_style_bg_color(my, lv_color_hex(0x00FF00), 0);
    lv_obj_set_style_bg_opa(my, LV_OPA_COVER, 0);

    // now set parent to the screen
    lv_obj_set_parent(my, scr);
    lv_obj_invalidate(my);
    lv_obj_mark_layout_as_dirty(my);

    lv_obj_invalidate(scr);
    lv_obj_mark_layout_as_dirty(scr);
    // also set a position within screen
    lv_obj_set_pos(my, 10, 10);
    for (int i = 0; i < 1000; i++) {
        g_display->tick();
        delay(10);
    }
    lv_obj_del(my);
    lv_obj_del(scr);
}

void test_draw_without_status_overlay(void) {
    lv_obj_t *screen = lv_obj_create(NULL);
    lv_obj_set_size(screen, 320, 240);
    lv_obj_set_style_bg_color(screen, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_center(screen);
    lv_screen_load(screen);
    ESP_LOGI("TEST", "Screen loaded");

    lv_obj_t *ui_containerOverlay = NULL;
    lv_obj_t *ui_btnAudio = NULL;
    lv_obj_t *ui_btnBLE = NULL;
    lv_obj_t *ui_btnWiFi = NULL;

    // draw a red rectangle in the center of the screen
    ui_containerOverlay = lv_obj_create(screen);
    lv_obj_remove_style_all(ui_containerOverlay);
    lv_obj_set_width(ui_containerOverlay, 66);
    lv_obj_set_height(ui_containerOverlay, 24);
    lv_obj_set_align(ui_containerOverlay, LV_ALIGN_TOP_RIGHT);
    lv_obj_set_flex_flow(ui_containerOverlay, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(ui_containerOverlay, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_remove_flag(ui_containerOverlay, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE); /// Flags
    lv_obj_set_style_pad_left(ui_containerOverlay, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_containerOverlay, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_containerOverlay, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_containerOverlay, 5, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_btnAudio = lv_button_create(ui_containerOverlay);
    lv_obj_set_width(ui_btnAudio, 10);
    lv_obj_set_height(ui_btnAudio, 10);
    lv_obj_set_align(ui_btnAudio, LV_ALIGN_CENTER);
    lv_obj_add_flag(ui_btnAudio, LV_OBJ_FLAG_SCROLL_ON_FOCUS); /// Flags
    lv_obj_remove_flag(ui_btnAudio, LV_OBJ_FLAG_SCROLLABLE);   /// Flags
    lv_obj_set_style_bg_color(ui_btnAudio, lv_color_hex(0xCE45DC), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_btnAudio, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_btnBLE = lv_button_create(ui_containerOverlay);
    lv_obj_set_width(ui_btnBLE, 10);
    lv_obj_set_height(ui_btnBLE, 10);
    lv_obj_set_align(ui_btnBLE, LV_ALIGN_CENTER);
    lv_obj_add_flag(ui_btnBLE, LV_OBJ_FLAG_SCROLL_ON_FOCUS); /// Flags
    lv_obj_remove_flag(ui_btnBLE, LV_OBJ_FLAG_SCROLLABLE);   /// Flags

    ui_btnWiFi = lv_button_create(ui_containerOverlay);
    lv_obj_set_width(ui_btnWiFi, 10);
    lv_obj_set_height(ui_btnWiFi, 10);
    lv_obj_set_align(ui_btnWiFi, LV_ALIGN_CENTER);
    lv_obj_add_flag(ui_btnWiFi, LV_OBJ_FLAG_SCROLL_ON_FOCUS); /// Flags
    lv_obj_remove_flag(ui_btnWiFi, LV_OBJ_FLAG_SCROLLABLE);   /// Flags
    lv_obj_set_style_bg_color(ui_btnWiFi, lv_color_hex(0x0ABF0F), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_btnWiFi, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    ESP_LOGI("TEST", "Overlay created");

    // lv_obj_set_parent(ui_containerOverlay, screen);

    ESP_LOGI("TEST", "Processing LVGL tasks...");
    for (int i = 0; i < 100; i++) {
        g_display->tick();
        delay(10);
    }
    delay(10000);
    // delete all the objects
    lv_obj_del(ui_containerOverlay);
    lv_obj_del(ui_btnAudio);
    lv_obj_del(ui_btnBLE);
    lv_obj_del(ui_btnWiFi);
    lv_obj_del(screen);
}

void test_status_overlay_manual_visual_check(void) {
    TEST_ASSERT_NOT_NULL(g_display);
    TEST_ASSERT_TRUE(g_display->isReady());

    StatusOverlay status;
    ESP_LOGI("TEST", "StatusOverlay created, calling initialize...");
    bool status_init_result = status.initialize();
    ESP_LOGI("TEST", "StatusOverlay initialize result: %s", status_init_result ? "true" : "false");
    TEST_ASSERT_TRUE_MESSAGE(status_init_result, "Status overlay initialize() failed");
    TEST_ASSERT_TRUE(status.isReady());

    // Please add a white screen, and show the status overlay on it.
    lv_obj_t *screen = lv_obj_create(NULL);
    lv_obj_set_size(screen, 320, 240);
    lv_obj_set_style_bg_color(screen, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_center(screen);

    // Load the screen to make it visible
    lv_screen_load(screen);
    ESP_LOGI("TEST", "Screen loaded");

    // Set size to match SquareLine Studio design (10x10 pixels)
    ESP_LOGI("TEST", "Setting indicator size...");
    status.setIndicatorSize(10);

    ESP_LOGI("TEST", "Attaching status overlay to screen...");
    status.attachToScreen(screen);

    ESP_LOGI("TEST", "Showing status overlay...");
    status.show();

    // Update status to make indicators visible with different colors
    ESP_LOGI("TEST", "Updating status indicators...");
    status.updateWiFiStatus(WiFiState::Working, "TestWiFi");
    status.updateBLEStatus(true, "NoKeyboard");
    status.updateAudioStatus(AudioState::Working, "TestSong.mp3");
    status.setWiFiBlinking(true);
    // Process LVGL tasks to ensure everything is rendered
    ESP_LOGI("TEST", "Processing LVGL tasks...");
    for (int i = 0; i < 1000; i++) {
        g_display->tick();
        status.tick();
        delay(10);
    }

    status.shutdown();
}