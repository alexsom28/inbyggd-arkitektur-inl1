/*
 * Project: Lightweight millisecond tracking library
 * Author: Zak Kemble, contact@zakkemble.net
 * Copyright: (C) 2018 by Zak Kemble
 * License: GNU GPL v3 (see License_GPL-3.0.txt) or MIT (see License_MIT.txt)
 * Web: http://blog.zakkemble.net/millisecond-tracking-library-for-avr/
 */

#ifndef MILLIS_H_
#define MILLIS_H_

typedef unsigned long millis_t;

#define MILLIS_TIMER0 0
#define MILLIS_TIMER1 1
#define MILLIS_TIMER2 2

#ifndef MILLIS_TIMER
  #define MILLIS_TIMER MILLIS_TIMER2
#endif

#ifndef ARDUINO
    #define millis() millis_get()
#endif

#ifdef __cplusplus
extern "C" {
#endif

void millis_init(void);
millis_t millis_get(void);
void millis_resume(void);
void millis_pause(void);
void millis_reset(void);
void millis_add(millis_t ms);
void millis_subtract(millis_t ms);

#ifdef __cplusplus
}
#endif

#endif /* MILLIS_H_ */

