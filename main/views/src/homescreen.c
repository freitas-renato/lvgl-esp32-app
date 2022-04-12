#include "lvgl.h"
#include "lvgl_helpers.h"

void homescreen_init(void) {
    lv_obj_t *scr = lv_obj_create(NULL, NULL);
    lv_scr_load(scr);
    lv_obj_t *label = lv_label_create(scr, NULL);
    lv_label_set_text(label, "Hello world!");
    lv_obj_align(label, NULL, LV_ALIGN_CENTER, 0, 0);
}
