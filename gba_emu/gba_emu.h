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
#ifndef LV_GBA_EMU_H
#define LV_GBA_EMU_H

#include "lvgl/lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*lv_gba_emu_input_update_cb_t)(bool* key_state_arr, uint16_t len);
typedef void (*lv_gba_emu_audio_output_cb_t)(const int16_t* data, size_t frames);

lv_obj_t* lv_gba_emu_create(lv_obj_t* par, const char* rom_file_path);
void lv_gba_emu_set_input_update_cb(lv_obj_t* gba_emu, lv_gba_emu_input_update_cb_t input_update_cb);
void lv_gba_emu_set_audio_output_cb(lv_obj_t* gba_emu, lv_gba_emu_audio_output_cb_t audio_output_cb);

#ifdef __cplusplus
}
#endif

#endif
