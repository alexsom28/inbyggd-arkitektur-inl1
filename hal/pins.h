#ifndef PINS_H
#define PINS_H

#include <avr/io.h>

/* ---- On-board LED (PB5 / Arduino pin 13) ---- */
#define LED_DDR   DDRB
#define LED_PORT  PORTB
#define LED_PIN   PB5

/* ---- Red LED (PD3 / Arduino pin 3) ---- */
#define RED_LED_DDR   DDRD
#define RED_LED_PORT  PORTD
#define RED_LED_PIN   PD3

/* ---- Green LED (PD2 / Arduino pin 2) ---- */
#define GREEN_LED_DDR   DDRD
#define GREEN_LED_PORT  PORTD
#define GREEN_LED_PIN   PD2

/* ---- Green push-button (PB4 / Arduino pin 12, active LOW to GND) ---- */
#define BTN_DDR   DDRB
#define BTN_PORT  PORTB
#define BTN_INPUT PINB
#define BTN_PIN   PB4

#endif
