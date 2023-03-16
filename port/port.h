/**
 * @file port.h
 *
 */

#ifndef PORT_H
#define PORT_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "lvgl/lvgl.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/

int lv_port_init(void);

void lv_port_sleep(uint32_t ms);

void gba_port_sdl_init(lv_obj_t* gba_emu);

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*PORT_H*/
