/**
 * @file lv_port_win.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "lv_drv_conf.h"

#if USE_WIN32DRV

#include "lv_port.h"
#include <windows.h>
#include "lv_drivers/win32drv/win32drv.h"

/*********************
 *      DEFINES
 *********************/
#ifdef LV_SCREEN_HOR_RES
#define SCREEN_HOR_RES  LV_SCREEN_HOR_RES
#else
#define SCREEN_HOR_RES  240
#endif

#ifdef LV_SCREEN_VER_RES
#define SCREEN_VER_RES  LV_SCREEN_VER_RES
#else
#define SCREEN_VER_RES  320
#endif

#define IDI_LVGL        101

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
    if (!lv_win32_init(
        GetModuleHandleW(NULL),
        SW_SHOW,
        SCREEN_HOR_RES,
        SCREEN_VER_RES,
        LoadIconW(GetModuleHandleW(NULL), (LPCWSTR)MAKEINTRESOURCE(IDI_LVGL))))
    {
        return -1;
    }

    lv_win32_add_all_input_devices_to_group(NULL);
    lv_timer_set_period(lv_anim_get_timer(), 16);
    lv_timer_set_period(lv_indev_get_read_timer(lv_win32_pointer_device_object), 16);
    return 0;
}

uint32_t lv_port_tick_get(void)
{
    return GetTickCount();
}

void lv_port_sleep(uint32_t ms)
{
    Sleep(ms);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

#endif /*USE_WIN32DRV*/
