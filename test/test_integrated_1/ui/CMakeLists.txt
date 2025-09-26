SET(SOURCES ui_Splash.c
    ui_Main.c
    ui_WIFI_Settings.c
    ui_Keyboard_Settings.c
    ui_Overlay.c
    ui.c
    ui_comp_hook.c
    ui_helpers.c
    ui_img_splash_clean_png.c)

add_library(ui ${SOURCES})
