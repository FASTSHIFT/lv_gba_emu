#ifndef __ST7789_H
#define __ST7789_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    uint8_t rst_pin;
    uint8_t cs_pin;
    uint8_t dc_pin;
    int16_t hor_res;
    int16_t ver_res;
    int16_t cur_width;
    int16_t cur_height;
} st7789_t;

int st7789_init(
    st7789_t* disp,
    uint8_t rst,
    uint8_t cs,
    uint8_t dc,
    int16_t hor_res,
    int16_t ver_res);
void st7789_set_addr_window(st7789_t* disp, int16_t x0, int16_t y0, int16_t x1, int16_t y1);
void st7789_set_rotation(st7789_t* disp, uint8_t r);
void st7789_fill_screen(st7789_t* disp, uint16_t color);
void st7789_draw_pixel(st7789_t* disp, int16_t x, int16_t y, uint16_t color);
void st7789_draw_bitmap(st7789_t* disp, int16_t x, int16_t y, const uint16_t* bitmap, int16_t w, int16_t h);

#ifdef __cplusplus
}
#endif

#endif /* __ST7789_H */
