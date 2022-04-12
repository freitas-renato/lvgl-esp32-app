/**
 * @file homescreen.c
 * @author Renato Freitas (freitas-renato@outlook.com)
 * @brief Home Screen View
 * @version 0.1
 * @date 2022-04-12
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "homescreen.h"
#include "lvgl.h"


/**** Image declares ****/

LV_IMG_DECLARE(logo_dither);


/**** Private screen variables ****/

static lv_style_t _screen_style;
static lv_obj_t* screen = NULL;


/**** Private screen functions ****/
static void move_logo_animation(uint32_t duration);


void homescreen_init(void) {
    // Init screen style
    lv_style_init(&_screen_style);
    lv_style_set_bg_color(&_screen_style, LV_STATE_DEFAULT, lv_color_hex(0x000000));
    lv_style_set_border_width(&_screen_style, LV_STATE_DEFAULT, 0);    
    lv_style_set_border_color(&_screen_style, LV_STATE_DEFAULT, lv_color_hex(0x000000));
    lv_style_set_radius(&_screen_style, LV_STATE_DEFAULT, 0);


    // Create blank screen with black background style
    screen = lv_obj_create(lv_scr_act(), NULL);
    lv_obj_set_size(screen, 240, 320);
    lv_obj_add_style(screen, LV_OBJ_PART_MAIN, &_screen_style);
    lv_scr_load(screen);

    // Place Instrutherm logo
    lv_obj_t* img = lv_img_create(screen, NULL);
    lv_img_set_src(img, &logo_dither);
    lv_obj_align(img, NULL, LV_ALIGN_IN_TOP_MID, 0, 40);


    move_logo_animation(1000);
}



static void move_logo_animation(uint32_t duration) {
    // Get image from current screen 
    lv_obj_t* img = lv_obj_get_child(screen, NULL);

    // Create animation
    lv_anim_t a;
    lv_anim_init(&a);

    lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_obj_set_y);
    lv_anim_set_var(&a, img);
    lv_anim_set_time(&a, duration);

    // Set animation play back to get the logo back to the top of the screen
    lv_anim_set_playback_delay(&a, 100);
    lv_anim_set_playback_time(&a, duration);

    // Get screen height
    lv_coord_t screen_height = lv_obj_get_height(screen);

    // Go to the bottom and come back up with 40px padding
    lv_anim_set_values(&a, 40, screen_height - 40 - lv_obj_get_height(img));

    // Repeat forever
    lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);

    // Start animation after 2s
    lv_anim_set_delay(&a, 2000);
    lv_anim_start(&a);
}
