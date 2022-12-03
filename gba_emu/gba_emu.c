/*
 * MIT License
 * Copyright (c) 2022 _VIFEXTech
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include "gba_emu.h"
#include "gba_internal.h"
#include "lvgl/lvgl.h"

static void gba_context_init(gba_context_t* ctx)
{
    LV_ASSERT_NULL(ctx);
    lv_memzero(ctx, sizeof(gba_context_t));
}

static void gba_emu_timer_cb(lv_timer_t* timer)
{
    gba_context_t* gba_ctx = timer->user_data;
    gba_retro_run(gba_ctx);
}

lv_obj_t* lv_gba_emu_create(lv_obj_t* par, const char* rom_file_path)
{
    LV_ASSERT_NULL(rom_file_path);

    static gba_context_t gba_ctx;
    gba_context_init(&gba_ctx);

    gba_retro_init(&gba_ctx);

    if (!gba_view_init(&gba_ctx, par)) {
        goto failed;
    }

    char real_path[128];
    lv_snprintf(real_path, sizeof(real_path), "/%s", rom_file_path);

    if (!gba_retro_load_game(&gba_ctx, real_path)) {
        LV_LOG_ERROR("load ROM: %s failed", real_path);
        goto failed;
    }

    gba_ctx.timer = lv_timer_create(gba_emu_timer_cb, 1000 / gba_ctx.av_info.fps, &gba_ctx);

failed:
    return gba_view_get_root(&gba_ctx);
}

void lv_gba_emu_set_input_update_cb(lv_obj_t* gba_emu, lv_gba_emu_input_update_cb_t input_update_cb)
{
    gba_context_t* ctx = lv_obj_get_user_data(gba_emu);
    LV_ASSERT_NULL(ctx);
    ctx->input_update_cb = input_update_cb;
}

void lv_gba_emu_set_audio_output_cb(lv_obj_t* gba_emu, lv_gba_emu_audio_output_cb_t audio_output_cb)
{
    gba_context_t* ctx = lv_obj_get_user_data(gba_emu);
    LV_ASSERT_NULL(ctx);
    ctx->audio_output_cb = audio_output_cb;
}
