#include "port.h"

#if LV_USE_RPI

#include "../gba_emu/gba_emu.h"
#include <alsa/asoundlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <wiringPi.h>

#define PCM_DEVICE "default"

#define AUDIO_FIFO_LEN 16384

typedef struct {
    volatile int16_t* buffer;
    volatile int head;
    volatile int tail;
    int size;
} audio_fifo_t;

typedef struct {
    pthread_t tid;
    sem_t flush_sem;
    sem_t wait_sem;
    snd_pcm_t* pcm_handle;
    audio_fifo_t audio_fifo;
    static int16_t audio_buffer[AUDIO_FIFO_LEN];
} audio_ctx_t;

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

static void* audio_thread(void* arg)
{
    audio_ctx_t* ctx = arg;
    int16_t buffer[AUDIO_FIFO_LEN];
    while (1) {
        sem_wait(&ctx->flush_sem);

        int avaliable = audio_fifo_avaliable(fifo);

        for (int i = 0; i < avaliable; i++) {
            buffer[i] = audio_fifo_read(fifo);
        }

        snd_pcm_sframes_t frames_written = snd_pcm_writei(pcm_handle, buffer, avaliable);
        if (frames_written < 0) {
            LV_LOG_ERROR("Write error: %s", snd_strerror(frames_written));
        } else if (frames_written != avaliable) {
            LV_LOG_WARN("Short write, expected %d frames but wrote %ld", avaliable, frames_written);
        }

        sem_post(&ctx->wait_sem);
    }
    return NULL;
}

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

static size_t gba_audio_output_cb(void* user_data, const int16_t* data, size_t frames)
{
    audio_ctx_t* ctx = user_data;
    bool wr_ok;
    int len = frames * 2;

    sem_wait(&ctx->wait_sem);

    for (int i = 0; i < len; i++) {
        wr_ok = audio_fifo_write(&ctx->fifo, data[i]);
    }

    if (!wr_ok) {
        LV_LOG_INFO("audio over run");
    }

    sem_post(&ctx->flush_sem);

    return frames;
}

static int gba_audio_init(lv_obj_t* gba_emu)
{
    static audio_ctx_t ctx;
    audio_fifo_init(&ctx.audio_fifo, ctx.audio_buffer, AUDIO_FIFO_LEN);

    /* Initialize ALSA PCM handle */
    int ret = snd_pcm_open(&ctx.pcm_handle, PCM_DEVICE, SND_PCM_STREAM_PLAYBACK, 0);
    if (ret < 0) {
        LV_LOG_ERROR("Failed to open PCM device: %s", PCM_DEVICE);
        return ret;
    }

    /* Set hardware parameters based on your audio file's parameters */
    snd_pcm_set_params(ctx.pcm_handle, SND_PCM_FORMAT_S16_LE, SND_PCM_ACCESS_RW_INTERLEAVED, 1, 48000, 1, 125000);

    sem_init(&ctx.flush_sem, 0, 0);
    sem_init(&ctx.wait_sem, 0, 0);
    pthread_create(&ctx.tid, NULL, audio_thread, &ctx);

    lv_gba_emu_set_audio_output_cb(gba_emu, gba_audio_output_cb, &ctx);

    return 0;
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

    gba_audio_init(gba_emu);
}

#endif
