/**
 * @file lv_port_rpi.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "lvgl/lvgl.h"

#if LV_USE_RPI

#include "port.h"
#include <time.h>
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

    return 0;
}

void lv_port_sleep(uint32_t ms)
{
    usleep(ms * 1000);
}

uint32_t lv_port_tick_get(void)
{
    struct timespec ts;
    uint32_t ms;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    ms = ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
    return ms;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

#endif /*USE_SDL*/
