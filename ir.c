#include <stdint.h>
#include <stdbool.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include "ir.h"

typedef enum {
  S_AWAIT_START_MARK,
  S_AWAIT_START_SPACE,
  S_AWAIT_BIT_MARK,
  S_AWAIT_BIT_SPACE
} State;
static volatile State state = S_AWAIT_START_MARK;
static volatile uint8_t bitNum;
static volatile uint16_t accumulator;
static volatile uint16_t value = 0x0000;

// Timer controls, to start, reset and stop the 2MHz timer
static inline void timerReset(void) {
  TCNT1 = 0;
}
static inline void timerStart(void) {
  timerReset();
  TCCR1B = _BV(WGM12) | _BV(CS11);  // CTC mode, prescaler 8 (2 MHz)
}
static inline void timerStop(void) {
  TCCR1B = 0x00;
}

// Pin status for the (active-low) detector signal
static inline bool pinAsserted(void) {
  return (PINC & _BV(7)) == 0;
}
static inline bool pinDeasserted(void) {
  return (PINC & _BV(7)) != 0;
}

// Blue LED controls (LEDs are wired active-low)
static inline void ledOn(void) {
  PORTD &= ~_BV(5);
}
static inline void ledOff(void) {
  PORTD |= _BV(5);
}

// This is called if things get out of sync. It'll try to re-sync at the next
// mark.
static inline void fsmReset(void) {
  timerStop();
  ledOff();
  value = 0x0000;
  state = S_AWAIT_START_MARK;
}

// This is called for more serious problems that *should* never happen. It
// lights the red LED, and it stays lit.
static inline void fsmError(void) {
  PORTD &= ~_BV(6);
  fsmReset();
}

// Pin interrupt fires on every rising and falling edge of PC7 (INT4).
ISR(INT4_vect) {
  switch (state) {
    // Called at the beginning of a start mark
    case S_AWAIT_START_MARK:
      if (pinAsserted()) {
        timerStart();
        state = S_AWAIT_START_SPACE;
      }
      break;

    // Called at the end of the start mark. We can figure out the mark's
    // duration to see whether it really is a start mark.
    case S_AWAIT_START_SPACE:
      if (pinDeasserted()) {
        if (TCNT1 > 2*1800) {
          // More than 1800us, therefore it was a start mark ("0" marks are 600us,
          // "1" marks are 1200us, "start" marks are 2400us)
          timerReset();
          ledOn();
          bitNum = accumulator = 0;
          state = S_AWAIT_BIT_MARK;
        } else {
          // It was not a start mark. That's OK, we were probably just unlucky
          // and started in the middle of a burst. Keep trying, eventually it'll
          // work.
          fsmReset();
        }
      } else {
        fsmError();
      }
      break;

    // Called at the beginning of a bit mark. The remote control sends 15 bits.
    case S_AWAIT_BIT_MARK:
      if (pinAsserted()) {
        timerReset();
        state = S_AWAIT_BIT_SPACE;
      } else {
        fsmError();
      }
      break;

    // Called at the end of a bit mark. We can figure out the mark's duration to
    // see whether it was a "0" mark or a "1" mark.
    case S_AWAIT_BIT_SPACE:
      if (pinDeasserted()) {
        const uint16_t t = TCNT1;
        if (t < 2*1800) {
          timerReset();
          ++bitNum;
          accumulator <<= 1;  // assume it was a "0" mark
          if (t > 2*900) {
            ++accumulator;    // nope, it was a "1" mark
          }
          if (bitNum == 15) {
            value = accumulator;
            state = S_AWAIT_START_MARK;  // got all 15 bits
          } else {
            state = S_AWAIT_BIT_MARK;
          }
        } else {
          fsmError();  // a start mark in the middle of a bunch of data bits
        }
      } else {
        fsmError();
      }
      break;
  }
}

// Timer interrupt fires 26ms from the last edge. This should happen only when
// a button is released.
ISR(TIMER1_COMPA_vect) {
  fsmReset();
}

// Allow the USB stuff to get access to the current button-code.
uint16_t getValue(void) {
  return value;
}

// Initialise pin, timer and LEDs
void irInit(void) {
  // LEDs
  DDRD |= _BV(5) | _BV(6);

  // INT4 config
  EICRB = _BV(ISC40); // generate interrupt on INT4 edges
  EIMSK = _BV(INT4);  // enable INT4 interrupt

  // Configure timer 1
  TCNT1 = 0;
  OCR1A = 51999;  // 26ms
  TIMSK1 = _BV(OCIE1A);
  TCCR1A = 0x00;
  timerStop();
}
