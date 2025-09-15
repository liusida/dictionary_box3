#include "lvgl_drive.h"
#include <Arduino.h>

lv_group_t *getDefaultGroup() {
    lv_group_t *default_group = lv_group_get_default();
    
    // If no default group exists, create one
    if (default_group == nullptr) {
        default_group = lv_group_create();
        lv_group_set_default(default_group);
    }

    return default_group;
}

void loadScreen(lv_obj_t *screen) {
    lv_disp_load_scr(screen);
    lv_group_t *default_group = getDefaultGroup();
    lv_group_remove_all_objs(default_group);
    delay(100);
}

void addObjectToDefaultGroup(lv_obj_t *obj) {
    if (obj == nullptr) {
        return; // Safety check
    }
    lv_group_t *default_group = getDefaultGroup();
    lv_group_add_obj(default_group, obj);
}
