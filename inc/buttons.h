#ifndef BUTTONS_H
#define BUTTONS_H

#include <stdbool.h>
#include <stdint.h>

typedef enum
{
    BUTTON_ID_1 = 0,
    BUTTON_ID_2,
    BUTTON_ID_3,
    BUTTON_ID_4,
    BUTTON_COUNT
} button_id_t;

#define BTN1_PIN 2
#define BTN2_PIN 3
#define BTN3_PIN 4
#define BTN4_PIN 5

void buttons_init(void);
void buttons_update(void);

bool button_is_pressed(button_id_t button);
bool button_was_pressed(button_id_t button);

uint32_t button_hold_ms(button_id_t button);
uint32_t buttons_get_mask(void);

#endif