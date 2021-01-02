#include <stdint.h>
#include <stdbool.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include "mouse.h"

#define COUNT_INIT 375  // 375*16ms = 6 seconds
//#define COUNT_INIT 4  // 4*16ms = 64ms
static volatile uint16_t count = COUNT_INIT;
static volatile bool sendReport = false;;

// Allow the USB stuff to know when a report is due (every six seconds)
bool mouseIsReportDue(void) {
  if (sendReport) {
    sendReport = false;
    return true;
  }
  return false;
}

// Detect button pressed
bool mouseIsButtonPressed(void) {
  return !(PIND & _BV(7));
}

// Timer interrupt fires every 16ms.
ISR(TIMER0_COMPA_vect) {
  --count;
  if (count == 0) {
    count = COUNT_INIT;
    sendReport = true;
  }
}

// Initialise button and timer
void mouseInit(void) {
  // Button
  DDRD &= ~_BV(7);

  // Configure timer 0
  TCNT0 = 0;
  OCR0A = 249;  // 16ms (250*1024/16MHz = 0.016s)
  TIMSK0 = _BV(OCIE0A);
  TCCR0A = _BV(WGM01);
  TCCR0B = _BV(CS02) | _BV(CS00);  // CTC mode, prescaler 1024 (15.625 kHz)
}
