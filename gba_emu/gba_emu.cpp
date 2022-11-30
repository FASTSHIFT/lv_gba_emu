#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gba_emu.h"

#include "lv_drivers/indev/evdev.h"
#include "lvgl/lvgl.h"

#include "gba.h"
#include "globals.h"
#include "libretro.h"

#define GBA_FB_STRIDE PIX_BUFFER_SCREEN_WIDTH
#define GBA_SCREEN_WIDTH 240
#define GBA_SCREEN_HEIGHT 160
#define GBA_KEY_NUM 12

typedef struct
{
    bool is_quit;
    bool is_frame_draw;
    uint32_t frame_cnt;
    lv_obj_t* canvas;
#if (LV_COLOR_DEPTH != 16)
    lv_color_t canvas_buf[GBA_SCREEN_WIDTH * GBA_SCREEN_HEIGHT];
#endif
} gba_context_t;

static gba_context_t gba_ctx;

static void gba_context_update_frame(gba_context_t* ctx)
{
    ctx->is_frame_draw = false;
    do {
        CPULoop();
    } while (!ctx->is_frame_draw);

    ctx->frame_cnt++;
}

static uint32_t gba_context_get_joykey(gba_context_t* ctx)
{
    lv_indev_data_t data;
    static lv_indev_drv_t indev_drv;
    /* "a", "b", "select", "start", "right", "left", "up", "down", "r", "l", "turbo", "menu" */
    uint32_t key_state = 0;

    if (indev_drv.type != LV_INDEV_TYPE_KEYPAD) {
        lv_indev_drv_init(&indev_drv);
        indev_drv.type = LV_INDEV_TYPE_KEYPAD;
    }

    evdev_read(&indev_drv, &data);

    static const uint32_t key_map[GBA_KEY_NUM] = {
        LV_KEY_ENTER,
        LV_KEY_BACKSPACE,
        LV_KEY_PREV,
        LV_KEY_NEXT,

        LV_KEY_RIGHT,
        LV_KEY_LEFT,
        LV_KEY_UP,
        LV_KEY_DOWN,

        0,
        0,
        0,
        0
    };

    // static const uint32_t key_map[GBA_KEY_NUM] = {
    //     KEY_A,
    //     KEY_B,
    //     KEY_SPACE,
    //     KEY_ENTER,
    //     KEY_RIGHT,
    //     KEY_LEFT,
    //     KEY_UP,
    //     KEY_DOWN,
    //     KEY_R,
    //     KEY_L,
    //     KEY_T,
    //     KEY_M
    // };

    for (int i = 0; i < GBA_KEY_NUM; i++) {
        if (data.key == key_map[i]) {
            if (data.state == LV_INDEV_STATE_PRESSED) {
                key_state |= 1 << i;
            }
        }
    }

    return key_state;
}

static void gba_video_refresh_cb(const void* data, unsigned width, unsigned height, size_t pitch)
{
#if (LV_COLOR_DEPTH != 16)
    const lv_color16_t* src = (const lv_color16_t*)data;
    lv_color_t* dst = gba_ctx.canvas_buf;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            dst->full = lv_color_make(src->ch.red << 3, src->ch.green << 2, src->ch.blue << 3).full;
            dst++;
            src++;
        }
        src += (GBA_FB_STRIDE - GBA_SCREEN_WIDTH);
    }
#endif
    lv_obj_invalidate(gba_ctx.canvas);
    gba_ctx.is_frame_draw = true;
}

static size_t gba_audio_batch_cb(const int16_t* data, size_t frames)
{
    return 0;
}

static void gba_input_poll_cb(void)
{
}

static int16_t gba_input_state_cb(unsigned port, unsigned device, unsigned index, unsigned id)
{
    return 0;
}

static void gba_context_init(gba_context_t* ctx)
{
    lv_memzero(ctx, sizeof(gba_context_t));

    retro_set_video_refresh(gba_video_refresh_cb);
    retro_set_audio_sample_batch(gba_audio_batch_cb);
    retro_set_input_poll(gba_input_poll_cb);
    retro_set_input_state(gba_input_state_cb);

    lv_obj_t* canvas = lv_canvas_create(lv_scr_act());

#if (LV_COLOR_DEPTH != 16)
    lv_canvas_set_buffer(canvas, ctx->canvas_buf, GBA_SCREEN_WIDTH, GBA_SCREEN_HEIGHT, LV_IMG_CF_TRUE_COLOR);
#endif

    lv_obj_set_style_outline_color(canvas, lv_palette_main(LV_PALETTE_BLUE), 0);
    lv_obj_set_style_outline_width(canvas, 5, 0);
    lv_obj_center(canvas);
    ctx->canvas = canvas;
}

void gba_emu_init()
{
    gba_context_init(&gba_ctx);

    const char* rom_file_path = "/rom/OpenLara.gba";
    int rom_size = CPULoadRom(rom_file_path);
    if (rom_size <= 0) {
        LV_LOG_ERROR("load ROM: %s failed", rom_file_path);
        return;
    }

    LV_LOG_USER("loaded ROM: %s, size = %d", rom_file_path, rom_size);

#if (LV_COLOR_DEPTH == 16)
    lv_canvas_set_buffer(gba_ctx.canvas, pix, GBA_FB_STRIDE, GBA_SCREEN_HEIGHT, LV_IMG_CF_TRUE_COLOR);
    lv_obj_set_width(gba_ctx.canvas, GBA_SCREEN_WIDTH);
    LV_LOG_USER("set direct canvas buffer = %p", pix);
#endif

    evdev_init();
    evdev_set_file((char*)"/dev/input/event1");

    CPUInit(NULL, false);
    CPUReset();
}

void gba_emu_loop()
{
    joy = gba_context_get_joykey(&gba_ctx);
    UpdateJoypad();
    gba_context_update_frame(&gba_ctx);
}
