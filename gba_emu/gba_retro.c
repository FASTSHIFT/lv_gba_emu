#include "gba_internal.h"
#include "libretro.h"
#include <math.h>

#define GBA_FB_STRIDE 256

static gba_context_t* gba_ctx_p = NULL;

static void retro_log_printf_cb(enum retro_log_level level, const char* fmt, ...)
{
    static const char* prompt[] = {
        "DEBUG", "INFO", "WARN", "ERROR"
    };

    if (level >= (sizeof(prompt) / sizeof(prompt[0]))) {
        return;
    }

    char buffer[128];

    va_list ap;
    va_start(ap, fmt);
    lv_vsnprintf(buffer, sizeof(buffer), fmt, ap);
    va_end(ap);

    LV_LOG("[RETRO][%s]: %s", prompt[level], buffer);
}

static bool retro_environment_cb(unsigned cmd, void* data)
{
    bool retval = true;
    switch (cmd) {
    case RETRO_ENVIRONMENT_GET_LOG_INTERFACE: {
        struct retro_log_callback* log = data;
        log->log = retro_log_printf_cb;
        break;
    }
    default:
        retval = false;
        break;
    }
    return retval;
}

static void retro_video_refresh_cb(const void* data, unsigned width, unsigned height, size_t pitch)
{
#if (LV_COLOR_DEPTH == 16)
    if (gba_ctx_p->canvas_buf != data) {
        gba_ctx_p->canvas_buf = (lv_color_t*)data;
        lv_canvas_set_buffer(gba_ctx_p->canvas, gba_ctx_p->canvas_buf, gba_ctx_p->fb_stride, height, LV_IMG_CF_TRUE_COLOR);
        lv_obj_set_width(gba_ctx_p->canvas, width);
        LV_LOG_USER("set direct canvas buffer = %p", gba_ctx_p->canvas_buf);
    }
#else
    const lv_color16_t* src = (const lv_color16_t*)data;
    lv_color_t* dst = gba_ctx_p->canvas_buf;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            dst->full = lv_color_make(src->ch.red << 3, src->ch.green << 2, src->ch.blue << 3).full;
            dst++;
            src++;
        }
        src += (gba_ctx_p->fb_stride - width);
    }
#endif
    lv_obj_invalidate(gba_ctx_p->canvas);
}

static void retro_audio_sample_cb(int16_t left, int16_t right)
{
}

static size_t retro_audio_sample_batch_cb(const int16_t* data, size_t frames)
{
    return 0;
}

static void retro_input_poll_cb(void)
{
    if (!gba_ctx_p->input_update_cb) {
        return;
    }
    gba_ctx_p->input_update_cb(gba_ctx_p);
}

static int16_t retro_input_state_cb(unsigned port, unsigned device, unsigned index, unsigned id)
{
    return gba_ctx_p->key_state[id];
}

bool gba_retro_init(gba_context_t* ctx)
{
    gba_ctx_p = ctx;
    retro_set_environment(retro_environment_cb);
    retro_set_video_refresh(retro_video_refresh_cb);
    retro_set_audio_sample(retro_audio_sample_cb);
    retro_set_audio_sample_batch(retro_audio_sample_batch_cb);
    retro_set_input_poll(retro_input_poll_cb);
    retro_set_input_state(retro_input_state_cb);
    retro_init();

    struct retro_system_av_info av_info;
    retro_get_system_av_info(&av_info);

    ctx->fb_stride = GBA_FB_STRIDE;
    ctx->fps = lround(av_info.timing.fps);

#if (LV_COLOR_DEPTH != 16)
    lv_coord_t width = av_info.geometry.base_width;
    lv_coord_t height = av_info.geometry.base_height;
    size_t buf_size = LV_IMG_BUF_SIZE_TRUE_COLOR(width, height);
    LV_ASSERT(buf_size > 0);

    ctx->canvas_buf = lv_malloc(buf_size);
    LV_ASSERT_MALLOC(ctx->canvas_buf);
    if (!ctx->canvas_buf) {
        retro_deinit();
        LV_LOG_ERROR("canvas_buf malloc failed");
        return false;
    }
    lv_canvas_set_buffer(ctx->canvas, ctx->canvas_buf, width, height, LV_IMG_CF_TRUE_COLOR);
#endif

    return true;
}

bool gba_retro_load_game(gba_context_t* ctx, const char* path)
{
    struct retro_game_info info;
    lv_memzero(&info, sizeof(info));
    info.path = path;
    return retro_load_game(&info);
}

void gba_retro_run(gba_context_t* ctx)
{
    retro_run();
}
