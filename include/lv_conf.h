/* src/lv_conf.h  (or move to include/) */
#ifndef LV_CONF_H
#define LV_CONF_H
// #warning "Using my own lv_conf.h"

#define LV_COLOR_DEPTH 16
// #define LV_COLOR_16_SWAP 1 // this is obsolete in LVGL 9.x

// Only enable fonts actually used in the project
#define LV_FONT_MONTSERRAT_12	0  // Not used
#define LV_FONT_MONTSERRAT_14	1  // Used: WiFi settings, keyboard settings, splash screen, default font
#define LV_FONT_MONTSERRAT_16	1  // Used: Main screen (word input, explanation, sample sentence)
#define LV_FONT_MONTSERRAT_18	0  // Not used
#define LV_FONT_MONTSERRAT_20	0  // Not used
#define LV_FONT_MONTSERRAT_22	0  // Not used
#define LV_FONT_MONTSERRAT_24	0  // Not used
#define LV_FONT_MONTSERRAT_26	0  // Not used
#define LV_FONT_MONTSERRAT_28	0  // Not used
#define LV_FONT_MONTSERRAT_30	0  // Not used
#define LV_FONT_MONTSERRAT_32	0  // Not used
#define LV_FONT_MONTSERRAT_34	0  // Not used
#define LV_FONT_MONTSERRAT_36	0  // Not used
#define LV_FONT_MONTSERRAT_38	0  // Not used
#define LV_FONT_MONTSERRAT_40	0  // Not used
#define LV_FONT_MONTSERRAT_42	0  // Not used
#define LV_FONT_MONTSERRAT_44	0  // Not used
#define LV_FONT_MONTSERRAT_46	0  // Not used
#define LV_FONT_MONTSERRAT_48	0  // Not used

#define LV_FONT_DEFAULT        &lv_font_montserrat_14

#define CONFIG_LV_USE_XML 0 // To save SRAM
#define LV_USE_XML 0
// #define LV_USE_DRAW_SW_COMPLEX_GRADIENTS 1

#define LV_USE_LODEPNG 1
#define LV_USE_FS_IF        1
#define LV_FS_IF_LITTLEFS  'S'    // choose the letter you want to use

#define LV_USE_LOG      1
#define LV_LOG_LEVEL    LV_LOG_LEVEL_TRACE

// Use custom memory allocator to save SRAM (use PSRAM for large allocations)
#define LV_USE_STDLIB_MALLOC  LV_STDLIB_CUSTOM
#define LV_MEM_CUSTOM_INCLUDE "drivers_display/lvgl_memory.h"
#define LV_MEM_CUSTOM_ALLOC   lvgl_malloc
#define LV_MEM_CUSTOM_FREE    lvgl_free
#define LV_MEM_CUSTOM_REALLOC lvgl_realloc

#define LV_USE_STDLIB_STRING  LV_STDLIB_CLIB
#define LV_USE_STDLIB_SPRINTF LV_STDLIB_CLIB

#endif /* LV_CONF_H */
