#include "display.h"
#include "font5x7.h"

#include <stdio.h>
#include <stdarg.h>

static uint8_t oled_buffer[OLED_BUF_SIZE];
#include "font5x7.h"

static uint8_t cursor_x = 0;
static uint8_t cursor_y = 0;
//--------------------------------------------------
// Отправка одной команды SSD1306
//--------------------------------------------------
void display_command(uint8_t cmd)
{
    uint8_t buffer[2];

    buffer[0] = 0x00; // Control byte: команда
    buffer[1] = cmd;

    i2c_write_blocking(OLED_I2C,
                       OLED_ADDR,
                       buffer,
                       2,
                       false);
}

//--------------------------------------------------
// Отправка массива данных
//--------------------------------------------------
void display_data(const uint8_t *data, size_t len)
{
    uint8_t buffer[17];

    buffer[0] = 0x40; // Control byte: данные

    while (len)
    {
        uint8_t count = (len > 16) ? 16 : len;

        for (uint8_t i = 0; i < count; i++)
            buffer[i + 1] = data[i];

        i2c_write_blocking(OLED_I2C,
                           OLED_ADDR,
                           buffer,
                           count + 1,
                           false);

        data += count;
        len -= count;
    }
}
//--------------------------------------------------
// Инициализация SSD1306
//--------------------------------------------------
void display_init(void)
{
    // Настройка I2C0 на 400 кГц
    i2c_init(OLED_I2C, 400000);

    // Перевод GPIO в режим I2C
    gpio_set_function(OLED_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(OLED_SCL_PIN, GPIO_FUNC_I2C);

    // Включаем подтяжки
    gpio_pull_up(OLED_SDA_PIN);
    gpio_pull_up(OLED_SCL_PIN);

    // ---------- Последовательность инициализации SSD1306 ----------

    display_command(0xAE); // Display OFF

    display_command(0xD5); // Clock divide ratio
    display_command(0x80);

    display_command(0xA8); // Multiplex ratio
    display_command(0x3F);

    display_command(0xD3); // Display offset
    display_command(0x00);

    display_command(0x40); // Start line = 0

    display_command(0x8D); // Charge Pump
    display_command(0x14);

    display_command(0x20); // Addressing Mode
    display_command(0x00); // Horizontal

    display_command(0xA1); // Segment remap

    display_command(0xC8); // COM scan direction

    display_command(0xDA); // COM Pins
    display_command(0x12);

    display_command(0x81); // Contrast
    display_command(0xCF);

    display_command(0xD9); // Pre-charge
    display_command(0xF1);

    display_command(0xDB); // VCOMH
    display_command(0x40);

    display_command(0xA4); // Resume RAM display

    display_command(0xA6); // Normal display

    display_command(0xAF); // Display ON

    display_clear();
    display_update();
}
//--------------------------------------------------
// Очистка буфера
//--------------------------------------------------
void display_clear(void)
{
    for (int i = 0; i < OLED_BUF_SIZE; i++)
    {
        oled_buffer[i] = 0x00;
    }
}
//--------------------------------------------------
// Передача буфера на дисплей
//--------------------------------------------------
void display_update(void)
{
    display_command(0x21);
    display_command(0);
    display_command(127);

    display_command(0x22);
    display_command(0);
    display_command(7);

    display_data(oled_buffer, OLED_BUF_SIZE);
}
//--------------------------------------------------
// Нарисовать один пиксель
//--------------------------------------------------
void display_draw_pixel(uint8_t x, uint8_t y, bool color)
{
    // Проверяем границы экрана
    if (x >= OLED_WIDTH || y >= OLED_HEIGHT)
        return;

    // Вычисляем индекс байта в буфере
    uint16_t index = x + (y / 8) * OLED_WIDTH;

    // Маска нужного бита
    uint8_t mask = 1 << (y % 8);

    if (color)
    {
        oled_buffer[index] |= mask;
    }
    else
    {
        oled_buffer[index] &= ~mask;
    }
}
//--------------------------------------------------
// Горизонтальная линия
//--------------------------------------------------
void display_draw_hline(uint8_t x,
                        uint8_t y,
                        uint8_t length,
                        bool color)
{
    for (uint8_t i = 0; i < length; i++)
    {
        display_draw_pixel(x + i, y, color);
    }
}
//--------------------------------------------------
// Вертикальная линия
//--------------------------------------------------
void display_draw_vline(uint8_t x,
                        uint8_t y,
                        uint8_t length,
                        bool color)
{
    for (uint8_t i = 0; i < length; i++)
    {
        display_draw_pixel(x, y + i, color);
    }
}
//--------------------------------------------------
// Установить курсор
//--------------------------------------------------
void display_set_cursor(uint8_t x, uint8_t y)
{
    cursor_x = x;
    cursor_y = y;
}
//--------------------------------------------------
// Вывод одного символа
//--------------------------------------------------
void display_draw_char(char c)
{
    if (c < 32 || c > 126)
        return;

    const uint8_t *glyph = font5x7[c - 32];

    for (uint8_t col = 0; col < 5; col++)
    {
        uint8_t line = glyph[col];

        for (uint8_t row = 0; row < 7; row++)
        {
            display_draw_pixel(
                cursor_x + col,
                cursor_y + row,
                (line >> row) & 0x01);
        }
    }

    cursor_x += 6;
}
//--------------------------------------------------
// Вывод строки
//--------------------------------------------------
void display_print(const char *str)
{
    while (*str)
    {
        display_draw_char(*str++);
    }
}
void display_printf(const char *format, ...)
{
    char buffer[64];

    va_list args;

    va_start(args, format);

    vsnprintf(buffer, sizeof(buffer), format, args);

    va_end(args);

    display_print(buffer);
}