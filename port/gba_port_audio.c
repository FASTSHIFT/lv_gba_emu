/**
 * @file gba_port_audio.c
 *
 */

/*********************
 *      INCLUDES
 *********************/

#include "../gba_emu/gba_emu.h"
#include "port.h"

#if LV_USE_SDL
#include <SDL2/SDL.h>
#else
#include <alsa/asoundlib.h>
#include <pthread.h>
#include <semaphore.h>
#endif

/*********************
 *      DEFINES
 *********************/

#define PCM_DEVICE "default"

#define AUDIO_FIFO_LEN 16384

#if LV_USE_SDL
#define AUDIO_LOCK() SDL_LockAudio()
#define AUDIO_UNLOCK() SDL_UnlockAudio()
#else
#define AUDIO_LOCK()
#define AUDIO_UNLOCK()
#endif

/**********************
 *      TYPEDEFS
 **********************/

typedef struct {
    int16_t* buffer;
    int head;
    int tail;
    int size;
} audio_fifo_t;

typedef struct {
    int sample_rate;
    audio_fifo_t fifo;
    int16_t buffer[AUDIO_FIFO_LEN];
#if LV_USE_SDL == 0
    snd_pcm_t* pcm_handle;
    pthread_t thread_id;
    volatile bool running;
#endif
} audio_ctx_t;

/**********************
 *  STATIC PROTOTYPES
 **********************/

static size_t gba_audio_output_cb(void* user_data, const int16_t* data, size_t frames);
static int audio_init(audio_ctx_t* ctx);
static void audio_deinit(audio_ctx_t* ctx);
static void audio_fifo_init(audio_fifo_t* fifo, int16_t* buffer, int size);

/**********************
 *  STATIC VARIABLES
 **********************/

static audio_ctx_t g_audio_ctx;

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

int gba_audio_init(lv_obj_t* gba_emu)
{
    int ret;
    audio_fifo_init(&g_audio_ctx.fifo, g_audio_ctx.buffer, AUDIO_FIFO_LEN);

    int sample_rate = lv_gba_emu_get_audio_sample_rate(gba_emu);
    LV_ASSERT(sample_rate > 0);
    g_audio_ctx.sample_rate = sample_rate;

    ret = audio_init(&g_audio_ctx);
    if (ret < 0) {
        return ret;
    }

    lv_gba_emu_set_audio_output_cb(gba_emu, gba_audio_output_cb, &g_audio_ctx);

    return 0;
}

void gba_audio_deinit(lv_obj_t* gba_emu)
{
    LV_UNUSED(gba_emu);
    audio_deinit(&g_audio_ctx);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

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

#if LV_USE_SDL

static void sdl_audio_callback(void* user_data, uint8_t* stream, int len)
{
    audio_ctx_t* ctx = user_data;
    audio_fifo_t* fifo = &ctx->fifo;
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

static int audio_init(audio_ctx_t* ctx)
{
    SDL_AudioSpec audio_spec;
    memset(&audio_spec, 0, sizeof(audio_spec));
    audio_spec.freq = ctx->sample_rate;
    audio_spec.format = AUDIO_S16SYS;
    audio_spec.channels = 2;
    audio_spec.samples = 1024;
    audio_spec.callback = sdl_audio_callback;
    audio_spec.userdata = ctx;

    int ret = SDL_OpenAudio(&audio_spec, NULL);
    if (ret != 0) {
        LV_LOG_ERROR("SDL_OpenAudio failed: %d", ret);
        return ret;
    }
    SDL_PauseAudio(0);

    return ret;
}

static void audio_deinit(audio_ctx_t* ctx)
{
    SDL_CloseAudio();
}

#else

static void* audio_thread(void* arg)
{
    audio_ctx_t* ctx = arg;
    int16_t buffer[AUDIO_FIFO_LEN];
    audio_fifo_t* fifo = &ctx->fifo;

    while (ctx->running) {
        int avaliable = audio_fifo_avaliable(fifo);

        if (avaliable > 0) {
            for (int i = 0; i < avaliable; i++) {
                buffer[i] = audio_fifo_read(fifo);
            }

            int channels = 2;
            int bytes_per_sample = 2; /* 16 bit */
            int buffer_size = avaliable * sizeof(int16_t);
            int frames = buffer_size / (channels * bytes_per_sample);
            snd_pcm_sframes_t frames_written;
            frames_written = snd_pcm_writei(ctx->pcm_handle, buffer, frames);
            if (frames_written < 0) {
                LV_LOG_ERROR("frames = %d, Write error: %s", frames, snd_strerror(frames_written));
                snd_pcm_recover(ctx->pcm_handle, frames_written, 0);
            } else if (frames_written != frames) {
                LV_LOG_WARN("Short write, expected %d frames but wrote %ld", avaliable, frames_written);
            }
        }

        usleep(100);
    }
    return NULL;
}

static int audio_init(audio_ctx_t* ctx)
{
    int ret;
    const char* device = getenv("LV_GBA_AUDIO_DEVICE");
    if (!device) {
        device = PCM_DEVICE;
    }

    /* Initialize ALSA PCM handle */
    ret = snd_pcm_open(&ctx->pcm_handle, device, SND_PCM_STREAM_PLAYBACK, 0);
    if (ret < 0) {
        LV_LOG_ERROR("Failed to open PCM device: %s, error: %s", device, snd_strerror(ret));
        return ret;
    }

    LV_LOG_USER("pcm_handle = %p, sample_rate = %d", ctx->pcm_handle, ctx->sample_rate);
    int channels = 2;

    /* Set hardware parameters based on your audio file's parameters */
    ret = snd_pcm_set_params(ctx->pcm_handle,
        SND_PCM_FORMAT_S16_LE,
        SND_PCM_ACCESS_RW_INTERLEAVED,
        channels,
        ctx->sample_rate,
        1,
        500000);
    if (ret < 0) {
        LV_LOG_ERROR("Unable to set PCM parameters: %s", snd_strerror(ret));
        snd_pcm_close(ctx->pcm_handle);
        ctx->pcm_handle = NULL;
        return ret;
    }

    ctx->running = true;
    ret = pthread_create(&ctx->thread_id, NULL, audio_thread, ctx);
    LV_ASSERT_MSG(ret == 0, "pthread_create failed");
    return ret;
}

static void audio_deinit(audio_ctx_t* ctx)
{
    if (ctx->running) {
        ctx->running = false;
        pthread_join(ctx->thread_id, NULL);
    }

    if (ctx->pcm_handle) {
        snd_pcm_close(ctx->pcm_handle);
        ctx->pcm_handle = NULL;
    }
}

#endif

static size_t gba_audio_output_cb(void* user_data, const int16_t* data, size_t frames)
{
    audio_ctx_t* ctx = user_data;
    int len = frames * 2;
    int written = 0;

    AUDIO_LOCK();

    while (len--) {
        if (!audio_fifo_write(&ctx->fifo, *data++)) {
            break;
        }
        written++;
    }

    AUDIO_UNLOCK();
    return written / 2;
}
