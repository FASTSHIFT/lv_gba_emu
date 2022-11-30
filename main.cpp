#include <stdio.h>
#include "lvgl/lvgl.h"
#include "lvgl/demos/lv_demos.h"
#include "lv_port/lv_port.h"
#include "gba_emu/gba_emu.h"

int main(int argc, const char* argv[])
{
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

    gba_emu_init();

    // lv_demo_widgets();

    while (true) {
        gba_emu_loop();
        lv_timer_handler();
        lv_port_sleep(1);
    }
}
