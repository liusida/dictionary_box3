#include "lvgl_drive.h"

void addObjectToDefaultGroup(lv_obj_t *obj) {
    if (obj == nullptr) {
        return; // Safety check
    }
    
    // Get the default group
    lv_group_t *default_group = lv_group_get_default();
    
    // If no default group exists, create one
    if (default_group == nullptr) {
        default_group = lv_group_create();
        lv_group_set_default(default_group);
    }
    
    // Add the object to the default group
    lv_group_add_obj(default_group, obj);
}
