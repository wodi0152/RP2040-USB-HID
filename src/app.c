#include "app.h"

#include <stdbool.h>
#include <stdint.h>

#include "pico/stdlib.h"

#include "buttons.h"
#include "device_mode.h"
#include "ui.h"
#include "usb_hid.h"

#define MENU_HOLD_TIME_MS 1000
#define DISPLAY_STARTUP_DELAY_MS 500

static ui_screen_t current_screen;
static device_mode_t active_mode;
static device_mode_t selected_mode;

static uint32_t previous_button_mask;
static bool previous_usb_state;

static bool menu_hold_handled;
static bool screen_dirty;

static void open_menu(void)
{
    current_screen = UI_SCREEN_MENU;
    selected_mode = active_mode;
    screen_dirty = true;
}

static void close_menu(void)
{
    current_screen = UI_SCREEN_STATUS;
    screen_dirty = true;
}

static void menu_move_up(void)
{
    if (selected_mode == DEVICE_MODE_KEYBOARD)
    {
        selected_mode = DEVICE_MODE_MACROPAD;
    }
    else
    {
        selected_mode--;
    }

    screen_dirty = true;
}

static void menu_move_down(void)
{
    selected_mode++;

    if (selected_mode >= DEVICE_MODE_COUNT)
    {
        selected_mode = DEVICE_MODE_KEYBOARD;
    }

    screen_dirty = true;
}

static void menu_select(void)
{
    active_mode = selected_mode;
    close_menu();
}

static void process_status_screen(void)
{
    /*
     * Долгое нажатие BTN4 открывает меню.
     */
    if (button_is_pressed(BUTTON_ID_4))
    {
        if (!menu_hold_handled &&
            button_hold_ms(BUTTON_ID_4) >= MENU_HOLD_TIME_MS)
        {
            menu_hold_handled = true;
            open_menu();
        }
    }
    else
    {
        menu_hold_handled = false;
    }
}

static void process_menu_screen(void)
{
    if (button_was_pressed(BUTTON_ID_1))
    {
        menu_move_up();
    }

    if (button_was_pressed(BUTTON_ID_2))
    {
        menu_move_down();
    }

    if (button_was_pressed(BUTTON_ID_3))
    {
        menu_select();
    }

    if (button_was_pressed(BUTTON_ID_4))
    {
        close_menu();
    }
}

static void update_screen(
    uint32_t button_mask,
    bool usb_connected)
{
    if (current_screen == UI_SCREEN_STATUS)
    {
        ui_render_status(
            button_mask,
            usb_connected,
            active_mode);
    }
    else
    {
        ui_render_menu(selected_mode);
    }

    screen_dirty = false;
}

void app_init(void)
{
    /*
     * Сначала запускаем USB и кнопки.
     */
    usb_hid_init();
    buttons_init();

    /*
     * После холодного включения SSD1306 может быть ещё
     * не готов принимать команды. Эта пауза позволяет
     * стабилизироваться питанию дисплея.
     */
    sleep_ms(DISPLAY_STARTUP_DELAY_MS);

    ui_init();

    current_screen = UI_SCREEN_STATUS;
    active_mode = DEVICE_MODE_KEYBOARD;
    selected_mode = DEVICE_MODE_KEYBOARD;

    previous_button_mask = UINT32_MAX;
    previous_usb_state = false;

    menu_hold_handled = false;
    screen_dirty = true;

    /*
     * Сразу рисуем первый экран после запуска.
     */
    update_screen(
        0,
        usb_hid_connected());
}

void app_task(void)
{
    buttons_update();

    uint32_t button_mask = buttons_get_mask();
    bool usb_connected = usb_hid_connected();

    if (current_screen == UI_SCREEN_STATUS)
    {
        process_status_screen();
    }
    else
    {
        process_menu_screen();
    }

    /*
     * Когда открыто меню, на компьютер не отправляются
     * команды клавиатуры.
     */
    uint32_t hid_button_mask =
        current_screen == UI_SCREEN_STATUS
            ? button_mask
            : 0;

    usb_hid_task(
        active_mode,
        hid_button_mask);

    /*
     * Перерисовываем OLED при изменении кнопок.
     */
    if (button_mask != previous_button_mask)
    {
        previous_button_mask = button_mask;
        screen_dirty = true;
    }

    /*
     * Перерисовываем OLED при изменении состояния USB.
     */
    if (usb_connected != previous_usb_state)
    {
        previous_usb_state = usb_connected;
        screen_dirty = true;
    }

    if (screen_dirty)
    {
        update_screen(
            button_mask,
            usb_connected);
    }

    sleep_ms(1);
}