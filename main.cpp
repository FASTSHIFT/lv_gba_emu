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
#include "gba_emu/gba_emu.h"
#include "lv_port/lv_port.h"
#include "lvgl/demos/lv_demos.h"
#include "lvgl/lvgl.h"
#include <stdio.h>

int main(int argc, const char* argv[])
{
    if (argc < 2) {
        printf("Please input rom file path.\neg: %s xxx.gba\n", argv[0]);
        return -1;
    }

    const char* rom_file_path = argv[1];

#if LV_USE_LOG
    lv_log_register_print_cb([](lv_log_level_t level, const char* str) {
        LV_UNUSED(level);
        printf("[LVGL]%s", str);
    });
#endif

    lv_init();

    if (lv_port_init() < 0) {
        LV_LOG_USER("hal init failed");
        return -1;
    }

    gba_emu_init(rom_file_path);

    // lv_demo_widgets();

    while (true) {
        gba_emu_loop();
        lv_timer_handler();
        lv_port_sleep(1);
    }
}
