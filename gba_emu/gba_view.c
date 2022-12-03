#include "gba_internal.h"

struct gba_view_s {
    lv_obj_t* root;

    struct {
        lv_obj_t* canvas;
        lv_color_t* buf;
    } screen;

    struct {
        struct {
            lv_obj_t* cont;
            lv_obj_t* up;
            lv_obj_t* down;
            lv_obj_t* left;
            lv_obj_t* right;
        } dir;

        struct {
            lv_obj_t* cont;
            lv_obj_t* A;
            lv_obj_t* B;
            lv_obj_t* L;
            lv_obj_t* R;
        } func;

        struct {
            lv_obj_t* cont;
            lv_obj_t* start;
            lv_obj_t* select;
        } ctrl;
    } btn;
};

typedef struct {
    const char* txt;
    lv_align_t align;
} btn_map_t;

static const btn_map_t btn_dir_map[] = {
    { LV_SYMBOL_UP, LV_ALIGN_TOP_MID },
    { LV_SYMBOL_DOWN, LV_ALIGN_BOTTOM_MID },
    { LV_SYMBOL_LEFT, LV_ALIGN_LEFT_MID },
    { LV_SYMBOL_RIGHT, LV_ALIGN_RIGHT_MID },
};

static const btn_map_t btn_func_map[] = {
    { "A", LV_ALIGN_LEFT_MID },
    { "B", LV_ALIGN_TOP_MID },
    { "L", LV_ALIGN_BOTTOM_MID },
    { "R", LV_ALIGN_RIGHT_MID },
};

static const btn_map_t btn_ctrl_map[] = {
    { "START", LV_ALIGN_LEFT_MID },
    { "SELECT", LV_ALIGN_RIGHT_MID },
};

static bool screen_create(gba_context_t* ctx)
{
    gba_view_t* view = ctx->view;

    lv_obj_t* canvas = lv_canvas_create(view->root);
    {
        view->screen.canvas = canvas;
        lv_obj_set_style_outline_color(canvas, lv_theme_get_color_primary(canvas), 0);
        lv_obj_set_style_outline_width(canvas, 5, 0);
    }

#if (LV_COLOR_DEPTH != 16)
    lv_coord_t width = ctx->av_info.fb_width;
    lv_coord_t height = ctx->av_info.fb_height;
    size_t buf_size = LV_IMG_BUF_SIZE_TRUE_COLOR(width, height);
    LV_ASSERT(buf_size > 0);

    view->screen.buf = lv_malloc(buf_size);
    LV_ASSERT_MALLOC(view->screen.buf);

    if (!view->screen.buf) {
        LV_LOG_ERROR("canvas buffer malloc failed");
        return false;
    }
    lv_canvas_set_buffer(view->screen.canvas, view->screen.buf, width, height, LV_IMG_CF_TRUE_COLOR);
#endif
    return true;
}

static void btn_event_cb(lv_event_t* e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_PRESSED || code == LV_EVENT_RELEASED) {
        lv_obj_t* btn = lv_event_get_current_target(e);
        bool* key_p = lv_event_get_user_data(e);
        *key_p = (code == LV_EVENT_PRESSED);
    }
}

static void btn_create(gba_context_t* ctx)
{
    gba_view_t* view = ctx->view;
#define CONT_SIZE 110
    {
        lv_obj_t* cont = lv_obj_create(view->root);

        view->btn.dir.cont = cont;

        lv_obj_set_size(cont, CONT_SIZE, CONT_SIZE);
        lv_obj_add_flag(cont, LV_OBJ_FLAG_FLEX_IN_NEW_TRACK);
        lv_obj_set_style_pad_all(cont, 0, 0);

        lv_obj_t** btn_arr = &view->btn.dir.up;
        for (int i = 0; i < GBA_ARRAY_SIZE(btn_dir_map); i++) {
            lv_obj_t* btn = lv_btn_create(cont);
            btn_arr[i] = btn;
            lv_obj_align(btn, btn_dir_map[i].align, 0, 0);

            lv_obj_t* label = lv_label_create(btn);
            lv_label_set_text(label, btn_dir_map[i].txt);
            lv_obj_center(label);
        }
    }

    {
        lv_obj_t* cont = lv_obj_create(view->root);

        view->btn.func.cont = cont;

        lv_obj_set_size(cont, CONT_SIZE, CONT_SIZE);
        lv_obj_set_style_pad_all(cont, 0, 0);

        lv_obj_t** btn_arr = &view->btn.func.A;
        for (int i = 0; i < GBA_ARRAY_SIZE(btn_func_map); i++) {
            lv_obj_t* btn = lv_btn_create(cont);
            btn_arr[i] = btn;
            lv_obj_align(btn, btn_func_map[i].align, 0, 0);

            lv_obj_t* label = lv_label_create(btn);
            lv_label_set_text(label, btn_func_map[i].txt);
            lv_obj_center(label);
        }
    }

    {
        lv_obj_t* cont = lv_obj_create(view->root);

        view->btn.ctrl.cont = cont;

        lv_obj_set_size(cont, CONT_SIZE * 2, LV_SIZE_CONTENT);
        lv_obj_add_flag(cont, LV_OBJ_FLAG_FLEX_IN_NEW_TRACK);
        lv_obj_set_style_pad_all(cont, 0, 0);

        lv_obj_t** btn_arr = &view->btn.ctrl.start;
        for (int i = 0; i < GBA_ARRAY_SIZE(btn_ctrl_map); i++) {
            lv_obj_t* btn = lv_btn_create(cont);
            btn_arr[i] = btn;
            lv_obj_align(btn, btn_ctrl_map[i].align, 0, 0);

            lv_obj_t* label = lv_label_create(btn);
            lv_label_set_text(label, btn_ctrl_map[i].txt);
            lv_obj_center(label);
        }
    }

#define BTN_EVENT_ATTACH(BTN, ID) \
    lv_obj_add_event_cb(view->btn.BTN, btn_event_cb, LV_EVENT_ALL, &ctx->key_state[GBA_JOYPAD_##ID]);

    BTN_EVENT_ATTACH(dir.up, UP);
    BTN_EVENT_ATTACH(dir.down, DOWN);
    BTN_EVENT_ATTACH(dir.left, LEFT);
    BTN_EVENT_ATTACH(dir.right, RIGHT);

    BTN_EVENT_ATTACH(func.A, A);
    BTN_EVENT_ATTACH(func.B, B);
    BTN_EVENT_ATTACH(func.L, L);
    BTN_EVENT_ATTACH(func.R, R);

    BTN_EVENT_ATTACH(ctrl.start, START);
    BTN_EVENT_ATTACH(ctrl.select, SELECT);
}

bool gba_view_init(gba_context_t* ctx, lv_obj_t* par)
{
    gba_view_t* view = lv_malloc(sizeof(gba_view_t));
    LV_ASSERT_MALLOC(view);
    ctx->view = view;

    lv_obj_t* root = lv_obj_create(par);
    {
        view->root = root;
        lv_obj_set_size(root, LV_PCT(100), LV_PCT(100));
        lv_obj_set_style_pad_all(root, 20, 0);
        lv_obj_set_flex_align(root, LV_FLEX_ALIGN_SPACE_AROUND, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_SPACE_AROUND);
    }

    bool retval = screen_create(ctx);

    btn_create(ctx);

    return retval;
}

lv_obj_t* gba_view_get_root(gba_context_t* ctx)
{
    LV_ASSERT_NULL(ctx);
    LV_ASSERT_NULL(ctx->view);
    return ctx->view->root;
}

void gba_view_draw_frame(gba_context_t* ctx, const uint16_t* buf, lv_coord_t width, lv_coord_t height)
{
    lv_obj_t* canvas = ctx->view->screen.canvas;
#if (LV_COLOR_DEPTH == 16)
    if (ctx->view->screen.buf != (lv_color_t*)buf) {
        ctx->view->screen.buf = (lv_color_t*)buf;
        lv_canvas_set_buffer(canvas, ctx->view->screen.buf, ctx->av_info.fb_stride, height, LV_IMG_CF_TRUE_COLOR);
        lv_obj_set_width(canvas, width);
        LV_LOG_USER("set direct canvas buffer = %p", ctx->view->screen.buf);
    }
#else
    const lv_color16_t* src = (const lv_color16_t*)buf;
    lv_color_t* dst = ctx->view->screen.buf;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            dst->full = lv_color_make(src->ch.red << 3, src->ch.green << 2, src->ch.blue << 3).full;
            dst++;
            src++;
        }
        src += (ctx->av_info.fb_stride - width);
    }
#endif
    lv_obj_invalidate(canvas);
}
