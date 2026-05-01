/*
 * Project: Lightweight millisecond tracking library
 * Author: Zak Kemble, contact@zakkemble.net
 * Copyright: (C) 2018 by Zak Kemble
 * License: GNU GPL v3 (see License_GPL-3.0.txt) or MIT (see License_MIT.txt)
 * Web: http://blog.zakkemble.net/millisecond-tracking-library-for-avr/
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/power.h>
#include <util/atomic.h>
#include "millis.h"

#ifndef F_CPU
    #error "F_CPU not defined!"
#endif

#if F_CPU < 256 || F_CPU >= 32640000
    #error "Bad F_CPU setting (<256 or >=32640000)"
#endif

#ifndef MILLIS_TIMER
    #error "Bad MILLIS_TIMER set"
#endif

#if MILLIS_TIMER == MILLIS_TIMER0

#if F_CPU > 16320000
    #define CLOCKSEL (_BV(CS02))
    #define PRESCALER 256
#elif F_CPU > 2040000
    #define CLOCKSEL (_BV(CS01)|_BV(CS00))
    #define PRESCALER 64
#elif F_CPU > 255
    #define CLOCKSEL (_BV(CS01))
    #define PRESCALER 8
#endif

#define REG_TCCRA   TCCR0A
#define REG_TCCRB   TCCR0B
#define REG_TIMSK   TIMSK0
#define REG_OCR     OCR0A
#define BIT_WGM     WGM01
#define BIT_OCIE    OCIE0A
#ifdef TIMER0_COMPA_vect
    #define ISR_VECT    TIMER0_COMPA_vect
#else
    #define ISR_VECT    TIM0_COMPA_vect
#endif
#define pwr_enable()    power_timer0_enable()
#define pwr_disable()   power_timer0_disable()

#define SET_TCCRA() (REG_TCCRA = _BV(BIT_WGM))
#define SET_TCCRB() (REG_TCCRB = CLOCKSEL)

#elif MILLIS_TIMER == MILLIS_TIMER1

#define CLOCKSEL (_BV(CS10))
#define PRESCALER 1

#define REG_TCCRA   TCCR1A
#define REG_TCCRB   TCCR1B
#define REG_TIMSK   TIMSK1
#define REG_OCR     OCR1A
#define BIT_WGM     WGM12
#define BIT_OCIE    OCIE1A
#ifdef TIMER1_COMPA_vect
    #define ISR_VECT    TIMER1_COMPA_vect
#else
    #define ISR_VECT    TIM1_COMPA_vect
#endif
#define pwr_enable()    power_timer1_enable()
#define pwr_disable()   power_timer1_disable()

#define SET_TCCRA() (REG_TCCRA = 0)
#define SET_TCCRB() (REG_TCCRB = _BV(BIT_WGM)|CLOCKSEL)

#elif MILLIS_TIMER == MILLIS_TIMER2

#if F_CPU > 16320000
    #define CLOCKSEL (_BV(CS22)|_BV(CS20))
    #define PRESCALER 128
#elif F_CPU > 8160000
    #define CLOCKSEL (_BV(CS22))
    #define PRESCALER 64
#elif F_CPU > 2040000
    #define CLOCKSEL (_BV(CS21)|_BV(CS20))
    #define PRESCALER 32
#elif F_CPU > 255
    #define CLOCKSEL (_BV(CS21))
    #define PRESCALER 8
#endif

#define REG_TCCRA   TCCR2A
#define REG_TCCRB   TCCR2B
#define REG_TIMSK   TIMSK2
#define REG_OCR     OCR2A
#define BIT_WGM     WGM21
#define BIT_OCIE    OCIE2A
#define ISR_VECT    TIMER2_COMPA_vect
#define pwr_enable()    power_timer2_enable()
#define pwr_disable()   power_timer2_disable()

#define SET_TCCRA() (REG_TCCRA = _BV(BIT_WGM))
#define SET_TCCRB() (REG_TCCRB = CLOCKSEL)

#else
    #error "Bad MILLIS_TIMER set"
#endif

static volatile millis_t milliseconds;

void millis_init()
{
    SET_TCCRA();
    SET_TCCRB();
    REG_TIMSK = _BV(BIT_OCIE);
    REG_OCR = ((F_CPU / PRESCALER) / 1000 - 1);
}

millis_t millis_get()
{
    millis_t ms;
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        ms = milliseconds;
    }
    return ms;
}

void millis_resume()
{
    pwr_enable();
    REG_TIMSK |= _BV(BIT_OCIE);
}

void millis_pause()
{
    REG_TIMSK &= ~_BV(BIT_OCIE);
    pwr_disable();
}

void millis_reset()
{
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        milliseconds = 0;
    }
}

void millis_add(millis_t ms)
{
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        milliseconds += ms;
    }
}

void millis_subtract(millis_t ms)
{
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        milliseconds -= ms;
    }
}

ISR(ISR_VECT)
{
    ++milliseconds;
}
