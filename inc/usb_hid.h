#ifndef USB_HID_H
#define USB_HID_H

#include <stdbool.h>
#include <stdint.h>

#include "device_mode.h"

void usb_hid_init(void);

void usb_hid_task(
    device_mode_t mode,
    uint32_t buttons);

bool usb_hid_connected(void);

#endif