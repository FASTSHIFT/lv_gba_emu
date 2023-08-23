#include "port.h"

#if LV_USE_SDL

#include "../gba_emu/gba_emu.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_keycode.h>

static uint32_t gba_input_update_cb(void* user_data)
{
    static const SDL_Keycode key_map[] = {
        SDL_SCANCODE_X, /* GBA_JOYPAD_B */
        0, /* GBA_JOYPAD_Y */
        SDL_SCANCODE_BACKSPACE, /* GBA_JOYPAD_SELECT */
        SDL_SCANCODE_TAB, /* GBA_JOYPAD_START */
        SDL_SCANCODE_UP, /* GBA_JOYPAD_UP */
        SDL_SCANCODE_DOWN, /* GBA_JOYPAD_DOWN */
        SDL_SCANCODE_LEFT, /* GBA_JOYPAD_LEFT */
        SDL_SCANCODE_RIGHT, /* GBA_JOYPAD_RIGHT */
        SDL_SCANCODE_Z, /* GBA_JOYPAD_A */
        0, /* GBA_JOYPAD_X */
        SDL_SCANCODE_L, /* GBA_JOYPAD_L */
        SDL_SCANCODE_R, /* GBA_JOYPAD_R */
        0, /* GBA_JOYPAD_L2 */
        0, /* GBA_JOYPAD_R2 */
        0, /* GBA_JOYPAD_L3 */
        0 /* GBA_JOYPAD_R3 */
    };

    const uint8_t* kbstate = SDL_GetKeyboardState(NULL);

    uint32_t key_state = 0;

    for (int i = 0; i < sizeof(key_map) / sizeof(key_map[0]); i++) {
        if (kbstate[key_map[i]]) {
            key_state |= (1 << i);
        }
    }

    return key_state;
}

void gba_port_init(lv_obj_t* gba_emu)
{
    lv_gba_emu_add_input_read_cb(gba_emu, gba_input_update_cb, NULL);
}

#endif
