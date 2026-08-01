#ifndef DISPLAY_H
#define DISPLAY_H

#include "pico/stdlib.h"
#include "hardware/i2c.h"

// I2C
#define OLED_I2C i2c0
#define OLED_SDA_PIN 0
#define OLED_SCL_PIN 1

// Адрес SSD1306
#define OLED_ADDR 0x3C

// Размер экрана
#define OLED_WIDTH 128
#define OLED_HEIGHT 64

// Размер буфера
#define OLED_BUF_SIZE (OLED_WIDTH * OLED_HEIGHT / 8)

void display_init(void);

void display_command(uint8_t cmd);

void display_data(const uint8_t *data, size_t len);

void display_update(void);

void display_clear(void);

void display_draw_pixel(uint8_t x, uint8_t y, bool color);
void display_draw_hline(uint8_t x,
                        uint8_t y,
                        uint8_t length,
                        bool color);

void display_draw_vline(uint8_t x,
                        uint8_t y,
                        uint8_t length,
                        bool color);
void display_set_cursor(uint8_t x, uint8_t y);

void display_draw_char(char c);

void display_print(const char *str);
void display_printf(const char *format, ...);
#endif