/*
 * MIT License
 * Copyright (c) 2022 - 2025 _VIFEXTech
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
#include "gba_emu/gba_emu.h"
#include "gba_emu/gba_menu.h"
#include "lvgl/lvgl.h"
#include "lvgl/src/misc/lv_profiler_builtin.h"
#include "port/port.h"
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define GBA_EMU_PREFIX "gba_emu: "

#define OPTARG_TO_VALUE(value, type, base)                                  \
    do {                                                                    \
        char* ptr;                                                          \
        (value) = (type)strtoul(optarg, &ptr, (base));                      \
        if (*ptr != '\0') {                                                 \
            printf(GBA_EMU_PREFIX "Parameter error: -%c %s\n", ch, optarg); \
            show_usage(argv[0], EXIT_FAILURE);                              \
        }                                                                   \
    } while (0)

typedef struct
{
    const char* file_path;
    const char* dir_path;
    lv_gba_view_mode_t mode;
    int volume;
    bool skip_intro;
    bool enable_profiler;
} gba_emu_param_t;

static void show_usage(const char* progname, int exitcode)
{
    printf("\nUsage: %s"
           " -f <string> -d <string> -m <decimal-value> -v <decimal-value> -s -h\n",
        progname);
    printf("\nWhere:\n");
    printf("  -f <string> rom file path.\n");
    printf("  -d <string> rom directory path (default: .).\n");
    printf("  -m <decimal-value> view mode: "
           "0: simple; 1: virtual keypad.\n");
    printf("  -v <decimal-value> set volume: 0 ~ 100.\n");
    printf("  -s skip intro animation.\n");
    printf("  -p enable profiler.\n");
    printf("  -h help.\n");

    exit(exitcode);
}

static void parse_commandline(int argc, char* const* argv, gba_emu_param_t* param)
{
    int ch;

    lv_memzero(param, sizeof(gba_emu_param_t));
    param->mode = LV_VER_RES < 400 ? LV_GBA_VIEW_MODE_SIMPLE : LV_GBA_VIEW_MODE_VIRTUAL_KEYPAD;
    param->volume = 100;
    param->dir_path = ".";
    param->skip_intro = false;

    while ((ch = getopt(argc, argv, "f:d:m:v:sph")) != -1) {
        switch (ch) {
        case 'f':
            param->file_path = optarg;
            break;

        case 'd':
            param->dir_path = optarg;
            break;

        case 'm':
            OPTARG_TO_VALUE(param->mode, lv_gba_view_mode_t, 10);
            break;

        case 'v':
            OPTARG_TO_VALUE(param->volume, int, 10);
            break;

        case 's':
            param->skip_intro = true;
            break;

        case 'p':
            param->enable_profiler = true;
            break;

        case '?':
            printf(GBA_EMU_PREFIX ": Unknown option: %c\n", optopt);
        case 'h':
            show_usage(argv[0], EXIT_FAILURE);
            break;
        }
    }
}

static void log_print_cb(lv_log_level_t level, const char* str)
{
    LV_UNUSED(level);
    printf("[LVGL]%s", str);
}

static void on_rom_selected(const char* path, void* user_data);

static void return_to_menu(void* user_data)
{
    gba_audio_deinit(NULL);
    gba_emu_param_t* param = (gba_emu_param_t*)user_data;
    lv_obj_clean(lv_scr_act());
    gba_menu_create(lv_scr_act(), param->dir_path, on_rom_selected, param);
}

static void on_game_exit(void* user_data)
{
    lv_async_call(return_to_menu, user_data);
}

static void on_rom_selected(const char* path, void* user_data)
{
    gba_emu_param_t* param = (gba_emu_param_t*)user_data;

    lv_obj_clean(lv_scr_act());

    lv_obj_t* gba_emu = lv_gba_emu_create(lv_scr_act(), path, param->mode);

    if (!gba_emu) {
        LV_LOG_USER("create gba emu failed");
        gba_menu_create(lv_scr_act(), param->dir_path, on_rom_selected, param);
        return;
    }

    lv_gba_emu_set_on_exit_cb(gba_emu, on_game_exit, param);

    gba_port_init(gba_emu);

    LV_LOG_USER("volume = %d", param->volume);
    if (param->volume > 0) {
        if (gba_audio_init(gba_emu) < 0) {
            LV_LOG_WARN("audio init failed");
        }
    }

    lv_obj_center(gba_emu);
}

static void intro_timer_cb(lv_timer_t* t)
{
    gba_emu_param_t* param = lv_timer_get_user_data(t);
    lv_obj_clean(lv_scr_act());

    if (param->file_path) {
        on_rom_selected(param->file_path, param);
    } else {
        gba_menu_create(lv_scr_act(), param->dir_path, on_rom_selected, param);
    }
}

static void obj_set_opacity_cb(lv_obj_t* obj, int32_t v)
{
    lv_obj_set_style_opa(obj, v, 0);
}

static lv_obj_t* create_intro_label(const char* text, uint32_t delay, uint32_t duration)
{
    lv_obj_t* label = lv_label_create(lv_scr_act());
    lv_label_set_text_static(label, text);

    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, label);
    lv_anim_set_values(&a, LV_OPA_TRANSP, LV_OPA_COVER);
    lv_anim_set_delay(&a, delay);
    lv_anim_set_time(&a, duration);
    lv_anim_set_early_apply(&a, true);
    lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)obj_set_opacity_cb);
    lv_anim_start(&a);

    return label;
}

static void start_intro(gba_emu_param_t* param)
{
#if LV_USE_PROFILER
    lv_profiler_builtin_set_enable(param->enable_profiler);
#endif

    if (param->skip_intro) {
        if (param->file_path) {
            on_rom_selected(param->file_path, param);
            return;
        }

        gba_menu_create(lv_scr_act(), param->dir_path, on_rom_selected, param);
        return;
    }

    lv_obj_t* label = create_intro_label("LVGL GBA Emulator", 0, 800);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_24, 0);
    lv_obj_center(label);

    label = create_intro_label("By _VIFEXTech", 800, 1600);
    lv_obj_align(label, LV_ALIGN_BOTTOM_RIGHT, -20, -50);

    lv_timer_t* timer = lv_timer_create(intro_timer_cb, 3000, param);
    lv_timer_set_repeat_count(timer, 1);
}

int main(int argc, const char* argv[])
{
#if LV_USE_LOG
    lv_log_register_print_cb(log_print_cb);
#endif

    lv_init();

    if (lv_port_init() < 0) {
        LV_LOG_USER("hal init failed");
        return -1;
    }

    static gba_emu_param_t param;
    parse_commandline(argc, (char* const*)argv, &param);
    start_intro(&param);

    while (true) {
        uint32_t sleep_ms = lv_timer_handler();
        lv_port_sleep(sleep_ms);
    }
}
