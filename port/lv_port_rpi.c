/**
 * @file lv_port_rpi.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "lvgl/lvgl.h"

#if LV_USE_RPI

#include "port.h"
#include "rpi/st7789.h"
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <wiringPi.h>

/*********************
 *      DEFINES
 *********************/

#define DISP_RST_PIN 27
#define DISP_CS_PIN 8
#define DISP_DC_PIN 25
#define DISP_BLK_PIN 24

#define KEY_UP_PIN 12
#define KEY_DOWN_PIN 20
#define KEY_LEFT_PIN 21
#define KEY_RIGHT_PIN 13
#define KEY_A_PIN 23
#define KEY_B_PIN 4
#define KEY_SELECT_PIN 16
#define KEY_START_PIN 26

#define HOR_RES 320
#define VER_RES 240

/**********************
 *      TYPEDEFS
 **********************/

typedef struct {
    pthread_t tid;
    sem_t flush_sem;
    sem_t wait_sem;
    st7789_t disp;
    int16_t x;
    int16_t y;
    const uint16_t* bitmap;
    int16_t w;
    int16_t h;
    uint16_t draw_buf1[HOR_RES * VER_RES];
    uint16_t draw_buf2[HOR_RES * VER_RES];
} disp_refr_ctx_t;

/**********************
 *  STATIC PROTOTYPES
 **********************/

static void disp_flush_cb(lv_disp_t* disp, const lv_area_t* area, lv_color_t* color_p);
static void disp_wait_cb(lv_disp_t* disp);
static void* disp_thread(void* arg);
static void keypad_read(lv_indev_t* indev, lv_indev_data_t* data);

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
int lv_port_init(void)
{
    int ret = wiringPiSetupGpio();
    if (ret < 0) {
        printf("wiringPiSetupGpio failed: %d\n", ret);
        return ret;
    }

    pinMode(DISP_BLK_PIN, OUTPUT);
    digitalWrite(DISP_BLK_PIN, 1);

    printf("display init...");

    static disp_refr_ctx_t ctx;
    memset(&ctx, 0, sizeof(ctx));
    ret = st7789_init(&ctx.disp, DISP_RST_PIN, DISP_CS_PIN, DISP_DC_PIN, HOR_RES, VER_RES);
    if (ret < 0) {
        printf("st7789_init failed: %d\n", ret);
        return ret;
    }
    printf("OK\n");

    st7789_set_rotation(&ctx.disp, 1);
    st7789_fill_screen(&ctx.disp, 0);

    sem_init(&ctx.flush_sem, 0, 0);
    sem_init(&ctx.wait_sem, 0, 0);
    pthread_create(&ctx.tid, NULL, disp_thread, &ctx);

    lv_disp_t* disp = lv_disp_create(HOR_RES, VER_RES);
    lv_disp_set_user_data(disp, &ctx);
    lv_disp_set_flush_cb(disp, disp_flush_cb);
    disp->wait_cb = disp_wait_cb;
    lv_disp_set_draw_buffers(
        disp,
        ctx.draw_buf1,
        ctx.draw_buf2,
        sizeof(ctx.draw_buf1),
        LV_DISP_RENDER_MODE_PARTIAL);

    /* Init keys */
    const int keys[] = { KEY_UP_PIN, KEY_DOWN_PIN, KEY_LEFT_PIN, KEY_RIGHT_PIN, KEY_A_PIN, KEY_B_PIN, KEY_SELECT_PIN, KEY_START_PIN };
    for (int i = 0; i < sizeof(keys) / sizeof(keys[0]); i++) {
        pinMode(keys[i], INPUT);
        pullUpDnControl(keys[i], PUD_UP);
    }

    /* Register indev */
    lv_indev_t* indev = lv_indev_create();
    lv_indev_set_type(indev, LV_INDEV_TYPE_KEYPAD);
    lv_indev_set_read_cb(indev, keypad_read);

    lv_group_set_default(lv_group_create());
    lv_indev_set_group(indev, lv_group_get_default());

    return 0;
}

void lv_port_sleep(uint32_t ms)
{
    usleep(ms * 1000);
}

uint32_t lv_port_tick_get(void)
{
    struct timespec ts;
    uint32_t ms;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    ms = ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
    return ms;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

static void* disp_thread(void* arg)
{
    disp_refr_ctx_t* ctx = arg;
    while (1) {
        sem_wait(&ctx->flush_sem);
        st7789_draw_bitmap(&ctx->disp, ctx->x, ctx->y, ctx->bitmap, ctx->w, ctx->h);
        sem_post(&ctx->wait_sem);
    }
    return NULL;
}

static void disp_flush(disp_refr_ctx_t* ctx, int16_t x, int16_t y, const uint16_t* bitmap, int16_t w, int16_t h)
{
    ctx->x = x;
    ctx->y = y;
    ctx->bitmap = bitmap;
    ctx->w = w;
    ctx->h = h;
    sem_post(&ctx->flush_sem);
}

static void keypad_read(lv_indev_t* indev, lv_indev_data_t* data)
{
    static uint32_t last_key = 0;
    uint32_t act_key = 0;

    if (digitalRead(KEY_UP_PIN) == 0) {
        act_key = LV_KEY_UP;
    } else if (digitalRead(KEY_DOWN_PIN) == 0) {
        act_key = LV_KEY_DOWN;
    } else if (digitalRead(KEY_LEFT_PIN) == 0) {
        act_key = LV_KEY_LEFT;
    } else if (digitalRead(KEY_RIGHT_PIN) == 0) {
        act_key = LV_KEY_RIGHT;
    } else if (digitalRead(KEY_A_PIN) == 0) {
        act_key = LV_KEY_ENTER;
    } else if (digitalRead(KEY_B_PIN) == 0) {
        act_key = LV_KEY_ESC;
    } else if (digitalRead(KEY_SELECT_PIN) == 0) {
        act_key = LV_KEY_BACKSPACE;
    } else if (digitalRead(KEY_START_PIN) == 0) {
        act_key = LV_KEY_ENTER;
    }

    if (act_key != 0) {
        data->state = LV_INDEV_STATE_PRESSED;
        last_key = act_key;
    } else {
        data->state = LV_INDEV_STATE_RELEASED;
    }

    data->key = last_key;
}

static void disp_flush_cb(lv_disp_t* disp, const lv_area_t* area, lv_color_t* color_p)
{
    lv_coord_t w = (area->x2 - area->x1 + 1);
    lv_coord_t h = (area->y2 - area->y1 + 1);

    disp_refr_ctx_t* ctx = lv_disp_get_user_data(disp);
    disp_flush(ctx, area->x1, area->y1, (uint16_t*)color_p, w, h);
}

static void disp_wait_cb(lv_disp_t* disp)
{
    disp_refr_ctx_t* ctx = lv_disp_get_user_data(disp);
    sem_wait(&ctx->wait_sem);
    lv_disp_flush_ready(disp);
}

#endif /*USE_SDL*/
