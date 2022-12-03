#include "gba_emu/gba_emu.h"
#include "lv_drivers/indev/evdev.h"

#if USE_EVDEV

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

void gba_port_init(lv_obj_t* gba_emu)
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
