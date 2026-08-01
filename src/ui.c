#include "ui.h"

#include <stdbool.h>
#include <stdint.h>

#include "device_mode.h"
#include "display.h"

/*
 * Преобразует режим в строку для OLED.
 */
static const char *mode_to_string(device_mode_t mode)
{
    switch (mode)
    {
    case DEVICE_MODE_KEYBOARD:
        return "KEYBOARD";

    case DEVICE_MODE_MACROPAD:
        return "MACROPAD";

    case DEVICE_MODE_MUSIC:
        return "MUSIC";

    default:
        return "UNKNOWN";
    }
}

/*
 * Вывод строки на выбранной текстовой линии.
 *
 * Высота символа вместе с промежутком составляет 8 пикселей.
 */
static void print_line(
    uint8_t line,
    const char *text)
{
    display_set_cursor(
        0,
        line * 8);

    display_printf(
        "%s",
        text);
}

/*
 * Вывод состояния физической кнопки.
 */
static void print_button_line(
    uint8_t line,
    const char *button_name,
    bool pressed)
{
    display_set_cursor(
        0,
        line * 8);

    display_printf(
        "%s: %s",
        button_name,
        pressed ? "ON " : "OFF");
}

void ui_init(void)
{
    display_init();
    display_clear();
    display_update();
}

void ui_render_status(
    uint32_t buttons,
    bool usb_connected,
    device_mode_t active_mode)
{
    display_clear();

    print_line(
        0,
        "RP2040USBHID");

    display_set_cursor(
        0,
        8);

    display_printf(
        "USB: %s",
        usb_connected
            ? "CONNECTED"
            : "WAIT...");

    display_set_cursor(
        0,
        16);

    display_printf(
        "MODE: %s",
        mode_to_string(active_mode));

    print_button_line(
        3,
        "BTN1",
        (buttons & (1u << 0)) != 0);

    print_button_line(
        4,
        "BTN2",
        (buttons & (1u << 1)) != 0);

    print_button_line(
        5,
        "BTN3",
        (buttons & (1u << 2)) != 0);

    print_button_line(
        6,
        "BTN4",
        (buttons & (1u << 3)) != 0);

    print_line(
        7,
        "HOLD BTN4: MENU");

    display_update();
}

void ui_render_menu(device_mode_t selected_mode)
{
    display_clear();

    print_line(
        0,
        "SELECT MODE");

    /*
     * Строка 1: Keyboard.
     */
    display_set_cursor(
        0,
        8);

    display_printf(
        "%c KEYBOARD",
        selected_mode == DEVICE_MODE_KEYBOARD
            ? '>'
            : ' ');

    /*
     * Строка 2: MacroPad.
     */
    display_set_cursor(
        0,
        16);

    display_printf(
        "%c MACROPAD",
        selected_mode == DEVICE_MODE_MACROPAD
            ? '>'
            : ' ');

    /*
     * Строка 3: Music.
     */
    display_set_cursor(
        0,
        24);

    display_printf(
        "%c MUSIC",
        selected_mode == DEVICE_MODE_MUSIC
            ? '>'
            : ' ');

    print_line(
        5,
        "BTN1 UP BTN2 DOWN");

    print_line(
        6,
        "BTN3 SELECT");

    print_line(
        7,
        "BTN4 BACK");

    display_update();
}