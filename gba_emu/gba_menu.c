/*
 * MIT License
 * Copyright (c) 2022 - 2025 _VIFEXTech
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include "gba_menu.h"

typedef struct {
    char base_path[512];
    gba_menu_select_cb_t cb;
    void* user_data;
} menu_ctx_t;

static menu_ctx_t g_menu_ctx;

static int is_gba_file(const char* filename)
{
    return strcmp(lv_fs_get_ext(filename), "gba") == 0;
}

static void event_handler(lv_event_t* e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t* btn = lv_event_get_target(e);

    if (code == LV_EVENT_CLICKED) {
        lv_obj_t* label = lv_obj_get_child(btn, 0);
        if (!label)
            return;

        const char* filename = lv_label_get_text(label);
        if (!filename)
            return;

        char full_path[1024];
        lv_snprintf(full_path, sizeof(full_path), "%s/%s", g_menu_ctx.base_path, filename);

        if (g_menu_ctx.cb) {
            g_menu_ctx.cb(full_path, g_menu_ctx.user_data);
        }
    }
}

void gba_menu_create(lv_obj_t* parent, const char* dir_path, gba_menu_select_cb_t cb, void* user_data)
{
    char fs_path[512];
    if (dir_path[0] != '/') {
        lv_snprintf(fs_path, sizeof(fs_path), "/%s", dir_path);
    } else {
        lv_snprintf(fs_path, sizeof(fs_path), "%s", dir_path);
    }

    strncpy(g_menu_ctx.base_path, fs_path, sizeof(g_menu_ctx.base_path) - 1);
    g_menu_ctx.cb = cb;
    g_menu_ctx.user_data = user_data;

    lv_obj_t* list = lv_list_create(parent);
    lv_obj_set_size(list, LV_PCT(100), LV_PCT(100));
    lv_obj_center(list);

    lv_list_add_text(list, "Select ROM");

    lv_fs_dir_t dir;
    lv_fs_res_t res;
    res = lv_fs_dir_open(&dir, fs_path);

    int count = 0;
    if (res == LV_FS_RES_OK) {
        char fn[256];
        while (1) {
            res = lv_fs_dir_read(&dir, fn);
            if (res != LV_FS_RES_OK || fn[0] == '\0')
                break;

            if (fn[0] == '/')
                continue;

            if (is_gba_file(fn)) {
                lv_obj_t* btn = lv_list_add_btn(list, NULL, fn);
                lv_obj_add_event(btn, event_handler, LV_EVENT_CLICKED, NULL);
                count++;
            }
        }
        lv_fs_dir_close(&dir);
    } else {
        lv_list_add_text(list, "Failed to open directory:");
        lv_list_add_text(list, fs_path);
    }

    if (count == 0 && res == LV_FS_RES_OK) {
        lv_list_add_text(list, "No .gba files found");
    }
}
