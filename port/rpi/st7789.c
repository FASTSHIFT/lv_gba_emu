#include "st7789.h"
#include <stdio.h>
#include <string.h>
#include <wiringPi.h>
#include <wiringPiSPI.h>

#define DISP_CMD_SET_X 0x2A
#define DISP_CMD_SET_Y 0x2B
#define DISP_CMD_WRITE_RAM 0x2C
#define DISP_CMD_READ_RAM 0x2D

#define DISP_USE_LITTLE_ENDIAN 1
#define DISP_SPI_CLK (60 * 1000000)
#define DISP_SPI_DATA_MAX_SIZE 4096

#define DISP_WIDTH_MAX 320

#define DISP_CS_SET digitalWrite(disp->cs_pin, 1)
#define DISP_CS_CLR digitalWrite(disp->cs_pin, 0)

#define DISP_DC_SET digitalWrite(disp->dc_pin, 1)
#define DISP_DC_CLR digitalWrite(disp->dc_pin, 0)

#define DISP_RST_SET digitalWrite(disp->rst_pin, 1)
#define DISP_RST_CLR digitalWrite(disp->rst_pin, 0)

#define DISP_WRITE_DATA(data) wiringPiSPIDataRW(0, &data, 1)
#define DISP_WRITE_DATA_BUF(buf, size) wiringPiSPIDataRW(0, (void*)buf, size)

#define DISP_LOG(fmt, ...) printf(fmt, ##__VA_ARGS__)

static void write_cmd(st7789_t* disp, uint8_t cmd)
{
    DISP_CS_CLR;
    DISP_DC_CLR;
    DISP_WRITE_DATA(cmd);
    DISP_CS_SET;
}

static void write_data(st7789_t* disp, uint8_t data)
{
    DISP_CS_CLR;
    DISP_DC_SET;
    DISP_WRITE_DATA(data);
    DISP_CS_SET;
}

static void write_data_buf(st7789_t* disp, const void* buf, size_t size)
{
    DISP_CS_CLR;
    DISP_DC_SET;

    const uint8_t* ptr = buf;

    while (size > DISP_SPI_DATA_MAX_SIZE) {
        DISP_WRITE_DATA_BUF(ptr, DISP_SPI_DATA_MAX_SIZE);
        ptr += DISP_SPI_DATA_MAX_SIZE;
        size -= DISP_SPI_DATA_MAX_SIZE;
    }
    DISP_WRITE_DATA_BUF(ptr, size);

    DISP_CS_SET;
}

int st7789_init(
    st7789_t* disp,
    uint8_t rst,
    uint8_t cs,
    uint8_t dc,
    int16_t hor_res,
    int16_t ver_res)
{
    memset(disp, 0, sizeof(st7789_t));
    disp->rst_pin = rst;
    disp->cs_pin = cs;
    disp->dc_pin = dc;
    disp->hor_res = hor_res;
    disp->ver_res = ver_res;
    disp->cur_width = hor_res;
    disp->cur_height = ver_res;

    pinMode(rst, OUTPUT);
    pinMode(cs, OUTPUT);
    pinMode(dc, OUTPUT);

    int clk = DISP_SPI_CLK;
    DISP_LOG("spi clock = %d\n", clk);
    int ret = wiringPiSPISetupMode(0, clk, 0);
    if (ret < 0) {
        DISP_LOG("wiringPiSPISetupMode failed: %d\n", ret);
        return ret;
    }

    DISP_RST_SET;
    delay(5);
    DISP_RST_CLR;
    delay(20);
    DISP_RST_SET;
    delay(150);

    /* command lists */
    write_cmd(disp, 0x11); //Sleep out
    delay(120);

    st7789_set_rotation(disp, 0);

    write_cmd(disp, 0x3A);
    write_data(disp, 0x05);

#if DISP_USE_LITTLE_ENDIAN
    /* Change to Little Endian */
    write_cmd(disp, 0xB0);
    write_data(disp, 0x00);  // RM = 0; DM = 00
    write_data(disp, 0xF8);  // EPF = 11; ENDIAN = 1; RIM = 0; MDT = 00 (ENDIAN -> 0 MSBFirst; 1 LSB First)
#endif

    write_cmd(disp, 0xB2);
    write_data(disp, 0x0C);
    write_data(disp, 0x0C);
    write_data(disp, 0x00);
    write_data(disp, 0x33);
    write_data(disp, 0x33);

    write_cmd(disp, 0xB7);
    write_data(disp, 0x35);

    write_cmd(disp, 0xBB);
    write_data(disp, 0x32); //Vcom=1.35V

    write_cmd(disp, 0xC2);
    write_data(disp, 0x01);

    write_cmd(disp, 0xC3);
    write_data(disp, 0x15); //GVDD=4.8V

    write_cmd(disp, 0xC4);
    write_data(disp, 0x20); //VDV, 0x20:0v

    write_cmd(disp, 0xC6);
    write_data(disp, 0x0F); //0x0F:60Hz

    write_cmd(disp, 0xD0);
    write_data(disp, 0xA4);
    write_data(disp, 0xA1);

    write_cmd(disp, 0xE0);
    write_data(disp, 0xD0);
    write_data(disp, 0x08);
    write_data(disp, 0x0E);
    write_data(disp, 0x09);
    write_data(disp, 0x09);
    write_data(disp, 0x05);
    write_data(disp, 0x31);
    write_data(disp, 0x33);
    write_data(disp, 0x48);
    write_data(disp, 0x17);
    write_data(disp, 0x14);
    write_data(disp, 0x15);
    write_data(disp, 0x31);
    write_data(disp, 0x34);

    write_cmd(disp, 0xE1);
    write_data(disp, 0xD0);
    write_data(disp, 0x08);
    write_data(disp, 0x0E);
    write_data(disp, 0x09);
    write_data(disp, 0x09);
    write_data(disp, 0x15);
    write_data(disp, 0x31);
    write_data(disp, 0x33);
    write_data(disp, 0x48);
    write_data(disp, 0x17);
    write_data(disp, 0x14);
    write_data(disp, 0x15);
    write_data(disp, 0x31);
    write_data(disp, 0x34);
    write_cmd(disp, 0x21);

    write_cmd(disp, 0x29);

    return 0;
}

void st7789_set_addr_window(st7789_t* disp, int16_t x0, int16_t y0, int16_t x1, int16_t y1)
{
    if (x0 < 0 || y0 < 0 || x1 < 0 || y1 < 0)
        return;

    uint8_t data_buf[4];
    write_cmd(disp, DISP_CMD_SET_X);
    data_buf[0] = x0 >> 8;
    data_buf[1] = x0;
    data_buf[2] = x1 >> 8;
    data_buf[3] = x1;
    write_data_buf(disp, data_buf, sizeof(data_buf));

    write_cmd(disp, DISP_CMD_SET_Y);
    data_buf[0] = y0 >> 8;
    data_buf[1] = y0;
    data_buf[2] = y1 >> 8;
    data_buf[3] = y1;
    write_data_buf(disp, data_buf, sizeof(data_buf));

    write_cmd(disp, DISP_CMD_WRITE_RAM);
}

void st7789_set_rotation(st7789_t* disp, uint8_t r)
{
    write_cmd(disp, 0x36);
    r %= 4;
    switch (r) {
    case 0:
        disp->cur_width = disp->hor_res;
        disp->cur_height = disp->ver_res;
        write_data(disp, 0x00);
        break;
    case 1:
        disp->cur_width = disp->ver_res;
        disp->cur_height = disp->hor_res;
        write_data(disp, 0xA0);
        break;
    case 2:
        disp->cur_width = disp->hor_res;
        disp->cur_height = disp->ver_res;
        write_data(disp, 0xC0);
        break;
    case 3:
        disp->cur_width = disp->ver_res;
        disp->cur_height = disp->hor_res;
        write_data(disp, 0x70);
        break;
    default:
        break;
    }
}

void st7789_fill_screen(st7789_t* disp, uint16_t color)
{
    uint8_t data;
    st7789_set_addr_window(disp, 0, 0, (disp->cur_width - 1), (disp->cur_height - 1));
    int16_t h = disp->cur_height;
    int16_t w = disp->cur_width;
    uint16_t buf[DISP_WIDTH_MAX];

    for (int x = 0; x < w; x++) {
        buf[x] = color;
    }

    while (h--) {
        write_data_buf(disp, buf, w * sizeof(uint16_t));
    }
}

void st7789_draw_pixel(st7789_t* disp, int16_t x, int16_t y, uint16_t color)
{
    if ((x < 0) || (x >= disp->cur_width) || (y < 0) || (y >= disp->cur_height))
        return;
    st7789_set_addr_window(disp, x, y, x + 1, y + 1);
    write_data(disp, color);
}

void st7789_draw_bitmap(st7789_t* disp, int16_t x, int16_t y, const uint16_t* bitmap, int16_t w, int16_t h)
{
    st7789_set_addr_window(disp, x, y, (x + w - 1), (y + h - 1));
    uint32_t size = w * h;
    write_data_buf(disp, bitmap, size * sizeof(uint16_t));
}
