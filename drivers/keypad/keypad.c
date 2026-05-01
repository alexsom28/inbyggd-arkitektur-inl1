#include "keypad.h"
#include "gpio.h"
#include <avr/io.h>
#include <util/delay.h>

/*
 * Wokwi wiring (from diagram.json):
 *   Rows:    R1=PB3(11), R2=PB2(10), R3=PB1(9), R4=PB0(8)
 *   Columns: C1=PD7(7),  C2=PD6(6),  C3=PD5(5), C4=PD4(4)
 *
 * Membrane keypad layout:
 *   C1  C2  C3  C4
 *   1   2   3   A    R1
 *   4   5   6   B    R2
 *   7   8   9   C    R3
 *   *   0   #   D    R4
 */

/* Row pins – all on PORTB */
static const uint8_t row_pins[KEYPAD_ROWS] = { PB3, PB2, PB1, PB0 };

/* Column pins – all on PORTD */
static const uint8_t col_pins[KEYPAD_COLS] = { PD7, PD6, PD5, PD4 };

/* Character map matching the standard 4×4 membrane keypad */
static const char key_map[KEYPAD_ROWS][KEYPAD_COLS] = {
    { '1', '2', '3', 'A' },
    { '4', '5', '6', 'B' },
    { '7', '8', '9', 'C' },
    { '*', '0', '#', 'D' }
};

void keypad_init(void)
{
    /* Rows: set as outputs, default HIGH (inactive) */
    for (uint8_t r = 0; r < KEYPAD_ROWS; r++)
    {
        gpio_pin_output(&DDRB, row_pins[r]);
        gpio_pin_high(&PORTB, row_pins[r]);
    }

    /* Columns: set as inputs with internal pull-ups */
    for (uint8_t c = 0; c < KEYPAD_COLS; c++)
    {
        gpio_pin_input_pullup(&DDRD, &PORTD, col_pins[c]);
    }
}

char keypad_scan(void)
{
    for (uint8_t r = 0; r < KEYPAD_ROWS; r++)
    {
        /* Drive current row LOW */
        gpio_pin_low(&PORTB, row_pins[r]);
        _delay_us(5);

        for (uint8_t c = 0; c < KEYPAD_COLS; c++)
        {
            /* Column reads LOW when a key in this row is pressed */
            if (!(PIND & (1U << col_pins[c])))
            {
                /* Simple debounce – wait and re-check */
                _delay_ms(20);
                if (!(PIND & (1U << col_pins[c])))
                {
                    /* Restore row before returning */
                    gpio_pin_high(&PORTB, row_pins[r]);

                    /* Wait for key release so we get one event per press */
                    while (!(PIND & (1U << col_pins[c])))
                    {
                        /* Also keep scanning the correct row */
                        gpio_pin_low(&PORTB, row_pins[r]);
                        _delay_ms(10);
                    }
                    gpio_pin_high(&PORTB, row_pins[r]);

                    return key_map[r][c];
                }
            }
        }

        /* Restore row to HIGH before moving to next */
        gpio_pin_high(&PORTB, row_pins[r]);
    }

    return KEYPAD_NO_KEY;
}
