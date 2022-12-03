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
#include "lvgl/lvgl.h"
#include <stdio.h>
#include <stdlib.h>

#if USE_EVDEV

#include "lv_drivers/indev/evdev.h"

static int get_kbd_event_number()
{
    const char* cmd = "grep -E 'Handlers|EV=' /proc/bus/input/devices | grep -B1 'EV=120013' | "
                      "grep -Eo 'event[0-9]+' | grep -Eo '[0-9]+' | tr -d '\n'";

    FILE* pipe = popen(cmd, "r");
    char buffer[8] = { 0 };
    int number = -1;
    while (!feof(pipe)) {
        if (fgets(buffer, sizeof(buffer), pipe) != NULL) {
            number = atoi(buffer);
        }
    }
    pclose(pipe);
    return number;
}

static void gba_input_update_cb(bool* key_state_arr, uint16_t len)
{
    lv_indev_data_t data;
    static lv_indev_drv_t indev_drv;

    if (indev_drv.type != LV_INDEV_TYPE_KEYPAD) {
        lv_indev_drv_init(&indev_drv);
        indev_drv.type = LV_INDEV_TYPE_KEYPAD;
    }

    evdev_read(&indev_drv, &data);

    static const lv_key_t key_map[] = {
        LV_KEY_BACKSPACE,
        0,
        LV_KEY_ESC,
        LV_KEY_HOME,
        LV_KEY_UP,
        LV_KEY_DOWN,
        LV_KEY_LEFT,
        LV_KEY_RIGHT,
        LV_KEY_ENTER,
        0,
        LV_KEY_PREV,
        LV_KEY_NEXT,
        0,
        0,
        0,
        0
    };

    for (int i = 0; i < len; i++) {
        if (data.key == key_map[i]) {
            key_state_arr[i] = (data.state == LV_INDEV_STATE_PRESSED);
        }
    }
}

static void gba_evdev_init(lv_obj_t* gba_emu)
{
    int number = get_kbd_event_number();
    if (number >= 0) {
        char kbd_event_name[32] = { 0 };
        lv_snprintf(kbd_event_name, sizeof(kbd_event_name), "/dev/input/event%d", number);
        LV_LOG_USER("kbd_name: %s", kbd_event_name);
        evdev_set_file(kbd_event_name);

        lv_gba_emu_set_input_update_cb(gba_emu, gba_input_update_cb);
    } else {
        LV_LOG_WARN("can't get kbd event number");
    }
}

#endif

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

    lv_obj_t* gba_emu = lv_gba_emu_create(lv_scr_act(), rom_file_path);
#if USE_EVDEV
    gba_evdev_init(gba_emu);
#endif

    while (true) {
        uint32_t sleep_ms = lv_timer_handler();
        sleep_ms = sleep_ms == 0 ? 1 : sleep_ms;
        lv_port_sleep(sleep_ms);
    }
}
