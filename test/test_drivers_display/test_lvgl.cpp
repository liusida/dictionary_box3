#include <Arduino.h>
#include <unity.h>
#include "display_manager.h"
#include "lvgl.h"
#include "test_splash_image.h"
#include "memory_test_helper.h"
#include "lvgl_helper.h"

// Helper function to safely cleanup LVGL objects
void safeCleanupLVGLObjects(lv_obj_t* obj1, lv_obj_t* obj2 = nullptr) {
    if (obj1) {
        lv_obj_del(obj1);
    }
    if (obj2) {
        lv_obj_del(obj2);
    }
}

// =================================== TESTS ===================================
void test_lvgl_create_and_load_screen(void) {
    DisplayManager disp;
    TEST_ASSERT_TRUE_MESSAGE(disp.initialize(), "Display initialize() failed");
    TEST_ASSERT_TRUE(disp.isReady());

    // Create a new screen
    lv_obj_t *scr = lv_obj_create(NULL);
    TEST_ASSERT_NOT_NULL(scr);
    lv_disp_load_scr(scr);
    
    // Verify the screen was loaded correctly
    lv_obj_t *cur = lv_screen_active();
    TEST_ASSERT_EQUAL_PTR(scr, cur);
    
    delay(10);
    // Run LVGL tick to process the display
    disp.tick();
    
    // Verify display is still ready
    TEST_ASSERT_TRUE(disp.isReady());
    
    // Clean up LVGL objects
    safeCleanupLVGLObjects(scr);
}

void test_lvgl_create_label_and_set_text(void) {
    DisplayManager disp;
    TEST_ASSERT_TRUE_MESSAGE(disp.initialize(), "Display initialize() failed");
    TEST_ASSERT_TRUE(disp.isReady());

    // Create a new screen (don't rely on active screen)
    lv_obj_t *scr = lv_obj_create(NULL);
    TEST_ASSERT_NOT_NULL(scr);
    lv_disp_load_scr(scr);

    // Create label on the screen
    lv_obj_t *label = lv_label_create(scr);
    TEST_ASSERT_NOT_NULL(label);
    
    // Set text content
    const char *msg = "Hello LVGL";
    lv_label_set_text(label, msg);
    
    // Verify the label object is valid
    TEST_ASSERT_NOT_NULL(lv_label_get_text(label));
    
    // Run LVGL tick to process the display
    disp.tick();
    
    // Verify display is still ready after showing label
    TEST_ASSERT_TRUE(disp.isReady());
    
    // Clean up LVGL objects
    safeCleanupLVGLObjects(label, scr);
}

void test_lvgl_display_png_image(void) {
    // Record initial memory state
    uint32_t initialHeap = ESP.getFreeHeap();
    uint32_t initialPsram = ESP.getFreePsram();
    
    DisplayManager disp;
    
    TEST_ASSERT_TRUE_MESSAGE(disp.initialize(), "Display initialize() failed");
    
    TEST_ASSERT_TRUE(disp.isReady());

    // Create a new screen
    lv_obj_t *scr = lv_obj_create(NULL);
    TEST_ASSERT_NOT_NULL(scr);
    
    lv_disp_load_scr(scr);

    // Create an image object
    lv_obj_t *img = lv_image_create(scr);
    TEST_ASSERT_NOT_NULL(img);
    
    // Set the PNG image data
    lv_image_set_src(img, &ui_img_splash_clean_png);
    
    // Center the image on the screen
    lv_obj_center(img);
    
    // Verify the image object is valid and has the correct source
    TEST_ASSERT_NOT_NULL(lv_image_get_src(img));
    
    // Run LVGL tick to process the display
    disp.tick();
    
    // Verify display is still ready after showing image
    TEST_ASSERT_TRUE(disp.isReady());
    
    // Clean up LVGL objects
    safeCleanupLVGLObjects(img, scr);
    
    disp.shutdown();
    // Check for memory leaks using the library function
    bool noLeaks = checkMemoryUsage("PNG Display", initialHeap, initialPsram);
    TEST_ASSERT_TRUE_MESSAGE(noLeaks, "Memory leak detected in PNG display test");
}

void test_lvgl_helper_getDefaultGroup(void) {
    DisplayManager disp;
    TEST_ASSERT_TRUE_MESSAGE(disp.initialize(), "Display initialize() failed");
    TEST_ASSERT_TRUE(disp.isReady());

    // Test getDefaultGroup function
    lv_group_t *group1 = getDefaultGroup();
    TEST_ASSERT_NOT_NULL(group1);
    
    // Test that subsequent calls return the same group
    lv_group_t *group2 = getDefaultGroup();
    TEST_ASSERT_EQUAL_PTR(group1, group2);
    
    // Test that the group is properly set as default
    lv_group_t *default_group = lv_group_get_default();
    TEST_ASSERT_EQUAL_PTR(group1, default_group);
    
    // Verify display is still ready
    TEST_ASSERT_TRUE(disp.isReady());
}

void test_lvgl_helper_loadScreen(void) {
    DisplayManager disp;
    TEST_ASSERT_TRUE_MESSAGE(disp.initialize(), "Display initialize() failed");
    TEST_ASSERT_TRUE(disp.isReady());

    // Create test screens
    lv_obj_t *screen1 = lv_obj_create(NULL);
    lv_obj_t *screen2 = lv_obj_create(NULL);
    TEST_ASSERT_NOT_NULL(screen1);
    TEST_ASSERT_NOT_NULL(screen2);
    
    // Test loadScreen function
    loadScreen(screen1);
    
    // Verify screen1 is now active
    lv_obj_t *active_screen = lv_screen_active();
    TEST_ASSERT_EQUAL_PTR(screen1, active_screen);
    
    // Test loading another screen
    loadScreen(screen2);
    
    // Verify screen2 is now active
    active_screen = lv_screen_active();
    TEST_ASSERT_EQUAL_PTR(screen2, active_screen);
    
    // Run LVGL tick to process the display
    disp.tick();
    
    // Verify display is still ready
    TEST_ASSERT_TRUE(disp.isReady());
    
    // Clean up
    safeCleanupLVGLObjects(screen1, screen2);
}

void test_lvgl_helper_addObjectToDefaultGroup(void) {
    DisplayManager disp;
    TEST_ASSERT_TRUE_MESSAGE(disp.initialize(), "Display initialize() failed");
    TEST_ASSERT_TRUE(disp.isReady());

    // Create a screen and some objects
    lv_obj_t *screen = lv_obj_create(NULL);
    lv_obj_t *button1 = lv_button_create(screen);
    lv_obj_t *button2 = lv_button_create(screen);
    lv_obj_t *label = lv_label_create(screen);
    
    TEST_ASSERT_NOT_NULL(screen);
    TEST_ASSERT_NOT_NULL(button1);
    TEST_ASSERT_NOT_NULL(button2);
    TEST_ASSERT_NOT_NULL(label);
    
    // Load the screen first
    loadScreen(screen);
    
    // Get the default group
    lv_group_t *group = getDefaultGroup();
    TEST_ASSERT_NOT_NULL(group);
    
    // Test adding objects to default group
    addObjectToDefaultGroup(button1);
    addObjectToDefaultGroup(button2);
    addObjectToDefaultGroup(label);
    
    // Verify objects were added to the group
    uint32_t group_size = lv_group_get_obj_count(group);
    TEST_ASSERT_EQUAL_UINT32(3, group_size);
    
    // Test adding null object (should not crash)
    addObjectToDefaultGroup(nullptr);
    
    // Group size should remain the same
    group_size = lv_group_get_obj_count(group);
    TEST_ASSERT_EQUAL_UINT32(3, group_size);
    
    // Run LVGL tick to process the display
    disp.tick();
    
    // Verify display is still ready
    TEST_ASSERT_TRUE(disp.isReady());
    
    // Clean up
    safeCleanupLVGLObjects(screen);
}

void test_lvgl_helper_integration(void) {
    DisplayManager disp;
    TEST_ASSERT_TRUE_MESSAGE(disp.initialize(), "Display initialize() failed");
    TEST_ASSERT_TRUE(disp.isReady());

    // Create multiple screens with objects
    lv_obj_t *screen1 = lv_obj_create(NULL);
    lv_obj_t *screen2 = lv_obj_create(NULL);
    
    lv_obj_t *button1 = lv_button_create(screen1);
    lv_obj_t *button2 = lv_button_create(screen1);
    lv_obj_t *label1 = lv_label_create(screen1);
    
    lv_obj_t *button3 = lv_button_create(screen2);
    lv_obj_t *label2 = lv_label_create(screen2);
    
    TEST_ASSERT_NOT_NULL(screen1);
    TEST_ASSERT_NOT_NULL(screen2);
    TEST_ASSERT_NOT_NULL(button1);
    TEST_ASSERT_NOT_NULL(button2);
    TEST_ASSERT_NOT_NULL(label1);
    TEST_ASSERT_NOT_NULL(button3);
    TEST_ASSERT_NOT_NULL(label2);
    
    // Test integrated workflow: load screen, add objects to group
    loadScreen(screen1);
    addObjectToDefaultGroup(button1);
    addObjectToDefaultGroup(button2);
    addObjectToDefaultGroup(label1);
    
    // Verify screen1 is active and objects are in group
    lv_obj_t *active_screen = lv_screen_active();
    TEST_ASSERT_EQUAL_PTR(screen1, active_screen);
    
    lv_group_t *group = getDefaultGroup();
    TEST_ASSERT_EQUAL_UINT32(3, lv_group_get_obj_count(group));
    
    // Switch to screen2 and add different objects
    loadScreen(screen2);
    addObjectToDefaultGroup(button3);
    addObjectToDefaultGroup(label2);
    
    // Verify screen2 is active and group was cleared and repopulated
    active_screen = lv_screen_active();
    TEST_ASSERT_EQUAL_PTR(screen2, active_screen);
    
    // Group should have been cleared by loadScreen and repopulated
    TEST_ASSERT_EQUAL_UINT32(2, lv_group_get_obj_count(group));
    
    // Run LVGL tick to process the display
    disp.tick();
    
    // Verify display is still ready
    TEST_ASSERT_TRUE(disp.isReady());
    
    // Clean up
    safeCleanupLVGLObjects(screen1, screen2);
}

void test_lvgl_helper_multiple_init_shutdown(void) {
    // Record initial memory state
    uint32_t initialHeap = ESP.getFreeHeap();
    uint32_t initialPsram = ESP.getFreePsram();
    
    ESP_LOGI("DisplayTest", "[Multiple Init/Shutdown] Initial: Heap=%u, PSRAM=%u", initialHeap, initialPsram);
    
    // First cycle: Initialize, load screen, shutdown
    {
        DisplayManager disp1;
        TEST_ASSERT_TRUE_MESSAGE(disp1.initialize(), "Display initialize() failed - cycle 1");
        TEST_ASSERT_TRUE(disp1.isReady());
        
        // Create and load a screen
        lv_obj_t *screen1 = lv_obj_create(NULL);
        TEST_ASSERT_NOT_NULL(screen1);
        loadScreen(screen1);
        
        // Verify screen is loaded
        lv_obj_t *active_screen = lv_screen_active();
        TEST_ASSERT_EQUAL_PTR(screen1, active_screen);
        
        // Run LVGL tick
        disp1.tick();
        
        // Clean up and shutdown
        safeCleanupLVGLObjects(screen1);
        disp1.shutdown();
        
        ESP_LOGI("DisplayTest", "[Multiple Init/Shutdown] After cycle 1: Heap=%u, PSRAM=%u", 
                 ESP.getFreeHeap(), ESP.getFreePsram());
    }
    
    // Second cycle: Initialize again, load different screen, shutdown
    {
        DisplayManager disp2;
        TEST_ASSERT_TRUE_MESSAGE(disp2.initialize(), "Display initialize() failed - cycle 2");
        TEST_ASSERT_TRUE(disp2.isReady());
        
        // Create and load a different screen
        lv_obj_t *screen2 = lv_obj_create(NULL);
        TEST_ASSERT_NOT_NULL(screen2);
        loadScreen(screen2);
        
        // Verify screen is loaded
        lv_obj_t *active_screen = lv_screen_active();
        TEST_ASSERT_EQUAL_PTR(screen2, active_screen);
        
        // Run LVGL tick
        disp2.tick();
        
        // Clean up and shutdown
        safeCleanupLVGLObjects(screen2);
        disp2.shutdown();
        
        ESP_LOGI("DisplayTest", "[Multiple Init/Shutdown] After cycle 2: Heap=%u, PSRAM=%u", 
                 ESP.getFreeHeap(), ESP.getFreePsram());
    }
    
    // Check for memory leaks
    bool noLeaks = checkMemoryUsage("Multiple Init/Shutdown", initialHeap, initialPsram);
    TEST_ASSERT_TRUE_MESSAGE(noLeaks, "Memory leak detected in multiple init/shutdown test");
}


