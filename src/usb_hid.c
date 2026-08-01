#include "usb_hid.h"

#include <stdbool.h>
#include <stdint.h>

#include "bsp/board_api.h"
#include "pico/stdlib.h"
#include "tusb.h"

#include "device_mode.h"
#include "usb_descriptors.h"

#define HID_REPORT_INTERVAL_MS 5

/*
 * Отправка стандартного клавиатурного отчёта.
 */
static void send_keyboard_report(
    uint8_t modifier,
    const uint8_t keycodes[6])
{
    tud_hid_keyboard_report(
        REPORT_ID_KEYBOARD,
        modifier,
        keycodes);
}

/*
 * Отпустить все обычные клавиши клавиатуры.
 */
static void release_keyboard(void)
{
    const uint8_t keycodes[6] = {0};

    send_keyboard_report(
        0,
        keycodes);
}

/*
 * Отправка Consumer Control отчёта:
 * громкость, Play/Pause, следующий трек и т. д.
 */
static void send_consumer_report(uint16_t usage)
{
    tud_hid_report(
        REPORT_ID_CONSUMER,
        &usage,
        sizeof(usage));
}

/*
 * Отпустить мультимедийную клавишу.
 */
static void release_consumer(void)
{
    send_consumer_report(0);
}

/*
 * Режим KEYBOARD:
 *
 * BTN1 -> Ctrl+C
 * BTN2 -> Ctrl+V
 * BTN3 -> Enter
 * BTN4 -> Esc
 */
static void send_keyboard_mode_report(uint32_t buttons)
{
    uint8_t modifier = 0;
    uint8_t keycodes[6] = {0};
    uint8_t index = 0;

    /*
     * BTN1 -> Ctrl+C
     */
    if ((buttons & (1u << 0)) != 0)
    {
        modifier |= KEYBOARD_MODIFIER_LEFTCTRL;

        if (index < 6)
        {
            keycodes[index++] = HID_KEY_C;
        }
    }

    /*
     * BTN2 -> Ctrl+V
     */
    if ((buttons & (1u << 1)) != 0)
    {
        modifier |= KEYBOARD_MODIFIER_LEFTCTRL;

        if (index < 6)
        {
            keycodes[index++] = HID_KEY_V;
        }
    }

    /*
     * BTN3 -> Enter
     */
    if ((buttons & (1u << 2)) != 0)
    {
        if (index < 6)
        {
            keycodes[index++] = HID_KEY_ENTER;
        }
    }

    /*
     * BTN4 -> Escape
     */
    if ((buttons & (1u << 3)) != 0)
    {
        if (index < 6)
        {
            keycodes[index++] = HID_KEY_ESCAPE;
        }
    }

    send_keyboard_report(
        modifier,
        keycodes);
}

/*
 * Режим MACROPAD:
 *
 * BTN1 -> Ctrl+S
 * BTN2 -> Ctrl+Z
 * BTN3 -> F5
 * BTN4 -> Shift+F5
 */
static void send_macropad_mode_report(uint32_t buttons)
{
    uint8_t modifier = 0;
    uint8_t keycodes[6] = {0};
    uint8_t index = 0;

    /*
     * BTN1 -> Ctrl+S
     */
    if ((buttons & (1u << 0)) != 0)
    {
        modifier |= KEYBOARD_MODIFIER_LEFTCTRL;

        if (index < 6)
        {
            keycodes[index++] = HID_KEY_S;
        }
    }

    /*
     * BTN2 -> Ctrl+Z
     */
    if ((buttons & (1u << 1)) != 0)
    {
        modifier |= KEYBOARD_MODIFIER_LEFTCTRL;

        if (index < 6)
        {
            keycodes[index++] = HID_KEY_Z;
        }
    }

    /*
     * BTN3 -> F5
     */
    if ((buttons & (1u << 2)) != 0)
    {
        if (index < 6)
        {
            keycodes[index++] = HID_KEY_F5;
        }
    }

    /*
     * BTN4 -> Shift+F5
     */
    if ((buttons & (1u << 3)) != 0)
    {
        modifier |= KEYBOARD_MODIFIER_LEFTSHIFT;

        if (index < 6)
        {
            keycodes[index++] = HID_KEY_F5;
        }
    }

    send_keyboard_report(
        modifier,
        keycodes);
}

/*
 * Режим MUSIC:
 *
 * BTN1 -> Volume -
 * BTN2 -> Play/Pause
 * BTN3 -> Volume +
 * BTN4 -> Next Track
 *
 * Consumer Control отчёт содержит одну команду.
 */
static void send_music_mode_report(uint32_t buttons)
{
    uint16_t usage = 0;

    if ((buttons & (1u << 0)) != 0)
    {
        usage = HID_USAGE_CONSUMER_VOLUME_DECREMENT;
    }
    else if ((buttons & (1u << 1)) != 0)
    {
        usage = HID_USAGE_CONSUMER_PLAY_PAUSE;
    }
    else if ((buttons & (1u << 2)) != 0)
    {
        usage = HID_USAGE_CONSUMER_VOLUME_INCREMENT;
    }
    else if ((buttons & (1u << 3)) != 0)
    {
        usage = HID_USAGE_CONSUMER_SCAN_NEXT;
    }

    send_consumer_report(usage);
}

/*
 * Отпускаем отчёт, использовавшийся предыдущим режимом.
 *
 * KEYBOARD и MACROPAD используют Keyboard Report.
 * MUSIC использует Consumer Report.
 */
static void release_mode_report(device_mode_t mode)
{
    if (mode == DEVICE_MODE_MUSIC)
    {
        release_consumer();
    }
    else
    {
        release_keyboard();
    }
}

void usb_hid_init(void)
{
    board_init();
    tusb_init();
}

bool usb_hid_connected(void)
{
    return tud_mounted();
}

void usb_hid_task(
    device_mode_t mode,
    uint32_t buttons)
{
    static absolute_time_t next_report_time;

    static uint32_t previous_buttons = UINT32_MAX;
    static device_mode_t previous_mode = DEVICE_MODE_KEYBOARD;

    /*
     * TinyUSB должен регулярно обрабатываться.
     */
    tud_task();

    /*
     * Компьютер ещё не распознал устройство.
     */
    if (!tud_mounted())
    {
        previous_buttons = UINT32_MAX;
        previous_mode = mode;
        return;
    }

    /*
     * Ограничиваем частоту отправки HID-отчётов.
     */
    if (!time_reached(next_report_time))
    {
        return;
    }

    next_report_time =
        make_timeout_time_ms(HID_REPORT_INTERVAL_MS);

    /*
     * HID-интерфейс пока не готов принять отчёт.
     */
    if (!tud_hid_ready())
    {
        return;
    }

    /*
     * При переключении режима сначала отпускаем
     * клавишу предыдущего режима.
     */
    if (mode != previous_mode)
    {
        release_mode_report(previous_mode);

        previous_mode = mode;
        previous_buttons = UINT32_MAX;
        return;
    }

    /*
     * Новый отчёт нужен только при изменении кнопок.
     *
     * При отпускании кнопки buttons становится равным 0,
     * поэтому отправляется пустой отчёт и клавиша
     * корректно отпускается.
     */
    if (buttons == previous_buttons)
    {
        return;
    }

    switch (mode)
    {
    case DEVICE_MODE_KEYBOARD:
        send_keyboard_mode_report(buttons);
        break;

    case DEVICE_MODE_MACROPAD:
        send_macropad_mode_report(buttons);
        break;

    case DEVICE_MODE_MUSIC:
        send_music_mode_report(buttons);
        break;

    default:
        release_keyboard();
        break;
    }

    previous_buttons = buttons;
}