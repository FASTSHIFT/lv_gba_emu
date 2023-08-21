#include "port.h"

#if LV_USE_RPI

#include "../gba_emu/gba_emu.h"
#include <wiringPi.h>

#define AUDIO_FIFO_LEN 16384

typedef struct {
    volatile int16_t* buffer;
    volatile int head;
    volatile int tail;
    int size;
} audio_fifo_t;

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
    uint32_t key_state = 0;

    for (int i = 0; i < sizeof(key_map) / sizeof(key_map[0]); i++) {
        int pin = key_map[i];
        if(pin < 0) {
            continue;
        }
        //printf("digitalRead(%d) = %d\n", pin, digitalRead(pin));
        if (digitalRead(pin) == LOW) {
            key_state |= (1 << i);
        }
    }

    return key_state;
}

static size_t gba_audio_output_cb(void* user_data, const int16_t* data, size_t frames)
{
    audio_fifo_t* fifo = user_data;
    bool wr_ok;
    int len = frames * 2;

    for (int i = 0; i < len; i++) {
        wr_ok = audio_fifo_write(fifo, data[i]);
    }

    if (!wr_ok) {
        LV_LOG_INFO("audio over run");
    }

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

    lv_gba_emu_set_audio_output_cb(gba_emu, gba_audio_output_cb, &audio_fifo);

    return 0;
}

void gba_port_init(lv_obj_t* gba_emu)
{
    for (int i = 0; i < sizeof(key_map) / sizeof(key_map[0]); i++) {
        int pin = key_map[i];
        if(pin < 0) {
            continue;
        }
        pinMode(pin, INPUT);
    }
    lv_gba_emu_add_input_read_cb(gba_emu, gba_input_update_cb, NULL);
    //gba_audio_init(gba_emu);
}

#endif
