/**
 * @file lv_port_sdl2.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "lvgl/lvgl.h"

#if LV_USE_SDL

#include "port.h"
#include <unistd.h>

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
int lv_port_init(void)
{
    lv_group_set_default(lv_group_create());

    lv_disp_t* disp = lv_sdl_window_create(LV_SCREEN_HOR_RES, LV_SCREEN_VER_RES);
    lv_indev_t* mouse = lv_sdl_mouse_create();
    lv_indev_set_group(mouse, lv_group_get_default());
    lv_indev_set_disp(mouse, disp);

    lv_indev_t* mousewheel = lv_sdl_mousewheel_create();
    lv_indev_set_disp(mousewheel, disp);
    lv_indev_set_group(mousewheel, lv_group_get_default());

    return 0;
}

void lv_port_sleep(uint32_t ms)
{
    usleep(ms * 1000);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

#endif /*USE_SDL*/
