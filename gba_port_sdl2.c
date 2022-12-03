#include "gba_emu/gba_emu.h"

#define USE_SDL 1

#if USE_SDL

#include <SDL2/SDL.h>
#include <SDL2/SDL_keycode.h>

static void gba_input_update_cb(bool* key_state_arr, uint16_t len)
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

    const int8_t* kbstate = SDL_GetKeyboardState(NULL);

    LV_ASSERT_NULL(kbstate);

    for (int i = 0; i < len; i++) {
        key_state_arr[i] = kbstate[key_map[i]];
    }
}

static size_t gba_audio_output_cb(const int16_t* data, size_t frames)
{
    return 0;
}

void gba_port_init(lv_obj_t* gba_emu)
{
    lv_gba_emu_set_input_update_cb(gba_emu, gba_input_update_cb);
    lv_gba_emu_set_audio_output_cb(gba_emu, gba_audio_output_cb);
}

#endif
