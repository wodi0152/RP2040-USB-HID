#ifndef UI_H
#define UI_H

#include <stdbool.h>
#include <stdint.h>

#include "device_mode.h"

typedef enum
{
    UI_SCREEN_STATUS = 0,
    UI_SCREEN_MENU
} ui_screen_t;

void ui_init(void);

void ui_render_status(
    uint32_t buttons,
    bool usb_connected,
    device_mode_t active_mode);

void ui_render_menu(device_mode_t selected_mode);

#endif