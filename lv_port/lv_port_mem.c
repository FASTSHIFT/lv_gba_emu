#include "lv_port.h"
#include "lvgl/lvgl.h"
#include "lvgl/src/misc/lv_malloc_builtin.h"
#include <stdlib.h>
#include <inttypes.h>

#define MEM_POLL_EXT_SIZE (8 * 1024 * 1024)

void lv_port_mem_init(void)
{
#if LV_USE_BUILTIN_MALLOC
    uint32_t buf_size = MEM_POLL_EXT_SIZE;
    void* buf = malloc(buf_size);
    LV_ASSERT_MALLOC(buf);

    LV_LOG_USER("add poll: address %p, size = %" PRIu32, buf, buf_size);
    lv_mem_builtin_add_pool(buf, buf_size);
#endif
}
