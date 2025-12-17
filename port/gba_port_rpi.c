#include "port.h"

#if LV_USE_RPI

#include "../gba_emu/gba_emu.h"
#include "rpi/wiring_pi_port.h"

static const int key_map[] = {
    4, /* GBA_JOYPAD_B */
    17, /* GBA_JOYPAD_Y */
    16, /* GBA_JOYPAD_SELECT */
    26, /* GBA_JOYPAD_START */
    12, /* GBA_JOYPAD_UP */
    20, /* GBA_JOYPAD_DOWN */
    21, /* GBA_JOYPAD_LEFT */
    13, /* GBA_JOYPAD_RIGHT */
    23, /* GBA_JOYPAD_A */
    22, /* GBA_JOYPAD_X */
    5, /* GBA_JOYPAD_L */
    6, /* GBA_JOYPAD_R */
    -1, /* GBA_JOYPAD_L2 */
    -1, /* GBA_JOYPAD_R2 */
    -1, /* GBA_JOYPAD_L3 */
    -1 /* GBA_JOYPAD_R3 */
};

static uint32_t gba_input_update_cb(void* user_data)
{
    uint32_t key_state = 0;

    for (int i = 0; i < sizeof(key_map) / sizeof(key_map[0]); i++) {
        int pin = key_map[i];

        if (pin < 0) {
            continue;
        }

        if (digitalRead(pin) == LOW) {
            key_state |= (1 << i);
        }
    }

    return key_state;
}

void gba_port_init(lv_obj_t* gba_emu)
{
    /* init pins */
    for (int i = 0; i < sizeof(key_map) / sizeof(key_map[0]); i++) {
        int pin = key_map[i];
        if (pin < 0) {
            continue;
        }

        /* input + pull up */
        pinMode(pin, INPUT);
        pullUpDnControl(pin, PUD_UP);
    }
    lv_gba_emu_add_input_read_cb(gba_emu, gba_input_update_cb, NULL);
}

#endif
