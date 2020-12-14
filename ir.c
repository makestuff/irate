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

static inline void resetTimer(void) {
  TCNT1 = 0;
}
static inline void startTimer(void) {
  resetTimer();
  TCCR1B = _BV(WGM12) | _BV(CS11);  // CTC mode, prescaler 8 (2 MHz)
}
static inline void stopTimer(void) {
  TCCR1B = 0x00;
}
static bool pinAsserted(void) {
  return (PINC & _BV(7)) == 0;
}
static bool pinDeasserted(void) {
  return (PINC & _BV(7)) != 0;
}
static void ledOn(void) {
  PORTD &= ~_BV(5);
}
static void ledOff(void) {
  PORTD |= _BV(5);
}
static void error(void) {
  PORTD &= ~_BV(6);
  state = S_AWAIT_START_MARK;
}

ISR(INT4_vect) {
  switch (state) {
  case S_AWAIT_START_MARK:
    if (pinAsserted()) {
      startTimer();
      state = S_AWAIT_START_SPACE;
    }
    break;

  case S_AWAIT_START_SPACE:
    if (pinDeasserted()) {
      if (TCNT1 > 2*1800) {
        // More than 1800us, therefore it was a start mark ("0" marks are 600us,
        // "1" marks are 1200us, "start" marks are 2400us)
        ledOn();
        bitNum = accumulator = 0;
        state = S_AWAIT_BIT_MARK;
      } else {
        // It was not a start mark
        state = S_AWAIT_START_MARK;
      }
    } else {
      error();
    }
    resetTimer();
    break;

  case S_AWAIT_BIT_MARK:
    if (pinAsserted()) {
      state = S_AWAIT_BIT_SPACE;
    } else {
      error();
    }
    resetTimer();
    break;

  case S_AWAIT_BIT_SPACE:
    if (pinDeasserted()) {
      state = S_AWAIT_BIT_MARK;
      ++bitNum;
      accumulator <<= 1;  // assume it was a "0" mark
      if (TCNT1 > 2*900) {
        ++accumulator;    // nope, it was a "1" mark
      }
      if (bitNum == 15) {
        value = accumulator;
        state = S_AWAIT_START_MARK;  // got all 15 bits
      } else {
        state = S_AWAIT_BIT_MARK;
      }
    } else {
      error();
    }
    resetTimer();
    break;
  }
}

ISR(TIMER1_COMPA_vect) {
  ledOff();
  value = 0;
  stopTimer();
  state = S_AWAIT_START_MARK;
}

uint16_t getValue(void) {
  return value;
}

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
  stopTimer();
}
