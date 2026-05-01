#ifndef KEYPAD_H
#define KEYPAD_H

#include <stdint.h>
#include <stdbool.h>

#define KEYPAD_ROWS 4
#define KEYPAD_COLS 4
#define KEYPAD_NO_KEY '\0'

/**
 * Initialise keypad GPIO pins.
 * Rows as outputs (directly driven low one at a time),
 * Columns as inputs with pull-ups.
 */
void keypad_init(void);

/**
 * Scan the keypad matrix and return the pressed key character.
 * Returns KEYPAD_NO_KEY if nothing is pressed.
 * Includes basic debounce.
 */
char keypad_scan(void);

#endif
