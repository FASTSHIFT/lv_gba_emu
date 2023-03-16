#include "port.h"
#include "../gba_emu/gba_emu.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_keycode.h>

#define AUDIO_FIFO_LEN 16384

typedef struct {
    volatile int16_t* buffer;
    volatile int head;
    volatile int tail;
    int size;
} audio_fifo_t;

static void audio_fifo_init(audio_fifo_t* fifo, int16_t* buffer, int size)
{
    memset(fifo, 0, sizeof(audio_fifo_t));
    fifo->buffer = buffer;
    fifo->size = size;
}

static bool audio_fifo_write(audio_fifo_t* fifo, int16_t data)
{
    int i = (fifo->head + 1) % fifo->size;

    if (i == fifo->tail) {
        return false;
    }

    fifo->buffer[fifo->head] = data;
    fifo->head = i;
    return true;
}

static int16_t audio_fifo_read(audio_fifo_t* fifo)
{
    if (fifo->head == fifo->tail) {
        return 0;
    }

    int16_t data = fifo->buffer[fifo->tail];
    fifo->tail = (fifo->tail + 1) % fifo->size;
    return data;
}

static int audio_fifo_avaliable(audio_fifo_t* fifo)
{
    return (fifo->size + fifo->head - fifo->tail) % fifo->size;
}

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

    const int8_t* kbstate = SDL_GetKeyboardState(NULL);

    uint32_t key_state = 0;

    for (int i = 0; i < sizeof(key_map) / sizeof(key_map[0]); i++) {
        if (kbstate[key_map[i]]) {
            key_state |= (1 << i);
        }
    }

    return key_state;
}

static size_t gba_audio_output_cb(void* user_data, const int16_t* data, size_t frames)
{
    audio_fifo_t* fifo = user_data;
    SDL_LockAudio();
    bool wr_ok;
    int len = frames * 2;

    for (int i = 0; i < len; i++) {
        wr_ok = audio_fifo_write(fifo, data[i]);
    }

    if (!wr_ok) {
        LV_LOG_INFO("audio over run");
    }

    SDL_UnlockAudio();
    return frames;
}

static void sdl_audio_callback(void* user_data, uint8_t* stream, int len)
{
    audio_fifo_t* fifo = user_data;
    uint16_t* wr_ptr = (uint16_t*)stream;

    int samples = len / 2;
    int avaliable = audio_fifo_avaliable(fifo);

    if (avaliable < samples) {
        LV_LOG_INFO("audio under run: %d < %d", avaliable, samples);
    }

    for (int i = 0; i < samples; i++) {
        *wr_ptr++ = audio_fifo_read(fifo);
    }
}

static int gba_audio_init(lv_obj_t* gba_emu)
{
    static audio_fifo_t audio_fifo;
    static int16_t audio_buffer[AUDIO_FIFO_LEN];
    audio_fifo_init(&audio_fifo, audio_buffer, AUDIO_FIFO_LEN);

    SDL_AudioSpec audio_spec;
    memset(&audio_spec, 0, sizeof(audio_spec));
    audio_spec.freq = lv_gba_emu_get_audio_sample_rate(gba_emu);
    audio_spec.format = AUDIO_S16SYS;
    audio_spec.channels = 2;
    audio_spec.samples = 1024;
    audio_spec.callback = sdl_audio_callback;
    audio_spec.userdata = &audio_fifo;

    int ret = SDL_OpenAudio(&audio_spec, NULL);
    if (ret != 0) {
        LV_LOG_ERROR("SDL_OpenAudio failed: %d", ret);
        return ret;
    }

    lv_gba_emu_set_audio_output_cb(gba_emu, gba_audio_output_cb, &audio_fifo);

    SDL_PauseAudio(0);
}

void gba_port_sdl_init(lv_obj_t* gba_emu)
{
    lv_gba_emu_add_input_read_cb(gba_emu, gba_input_update_cb, NULL);
    gba_audio_init(gba_emu);
}
