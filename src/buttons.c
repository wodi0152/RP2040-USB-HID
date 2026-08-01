#include "buttons.h"

#include "pico/stdlib.h"

#define DEBOUNCE_MS 20

typedef struct
{
    bool raw_state;
    bool stable_state;
    bool previous_stable_state;

    uint32_t raw_change_time_ms;
    uint32_t press_time_ms;
} button_state_t;

static const uint button_pins[BUTTON_COUNT] =
    {
        BTN1_PIN,
        BTN2_PIN,
        BTN3_PIN,
        BTN4_PIN};

static button_state_t button_states[BUTTON_COUNT];

static uint32_t get_time_ms(void)
{
    return to_ms_since_boot(get_absolute_time());
}

void buttons_init(void)
{
    uint32_t now = get_time_ms();

    for (uint8_t i = 0; i < BUTTON_COUNT; i++)
    {
        gpio_init(button_pins[i]);
        gpio_set_dir(button_pins[i], GPIO_IN);
        gpio_pull_up(button_pins[i]);

        bool pressed = gpio_get(button_pins[i]) == 0;

        button_states[i].raw_state = pressed;
        button_states[i].stable_state = pressed;
        button_states[i].previous_stable_state = pressed;
        button_states[i].raw_change_time_ms = now;
        button_states[i].press_time_ms = now;
    }
}

void buttons_update(void)
{
    uint32_t now = get_time_ms();

    for (uint8_t i = 0; i < BUTTON_COUNT; i++)
    {
        button_state_t *state = &button_states[i];

        /*
         * Сохраняем состояние предыдущего цикла.
         * Оно используется для определения момента нажатия.
         */
        state->previous_stable_state = state->stable_state;

        bool raw_pressed = gpio_get(button_pins[i]) == 0;

        /*
         * Обнаружено физическое изменение контакта.
         */
        if (raw_pressed != state->raw_state)
        {
            state->raw_state = raw_pressed;
            state->raw_change_time_ms = now;
        }

        /*
         * Принимаем новое состояние только после DEBOUNCE_MS.
         */
        if ((state->stable_state != state->raw_state) &&
            ((now - state->raw_change_time_ms) >= DEBOUNCE_MS))
        {
            state->stable_state = state->raw_state;

            if (state->stable_state)
            {
                state->press_time_ms = now;
            }
        }
    }
}

bool button_is_pressed(button_id_t button)
{
    if (button >= BUTTON_COUNT)
    {
        return false;
    }

    return button_states[button].stable_state;
}

bool button_was_pressed(button_id_t button)
{
    if (button >= BUTTON_COUNT)
    {
        return false;
    }

    return button_states[button].stable_state &&
           !button_states[button].previous_stable_state;
}

uint32_t button_hold_ms(button_id_t button)
{
    if (button >= BUTTON_COUNT)
    {
        return 0;
    }

    if (!button_states[button].stable_state)
    {
        return 0;
    }

    return get_time_ms() - button_states[button].press_time_ms;
}

uint32_t buttons_get_mask(void)
{
    uint32_t mask = 0;

    if (button_is_pressed(BUTTON_ID_1))
    {
        mask |= (1u << 0);
    }

    if (button_is_pressed(BUTTON_ID_2))
    {
        mask |= (1u << 1);
    }

    if (button_is_pressed(BUTTON_ID_3))
    {
        mask |= (1u << 2);
    }

    if (button_is_pressed(BUTTON_ID_4))
    {
        mask |= (1u << 3);
    }

    return mask;
}