#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/power.h>
#include <avr/interrupt.h>
#include <LUFA/Drivers/USB/USB.h>
#include "ir.h"
#include "usb.h"

int main(void) {
  MCUSR &= ~(1 << WDRF);
  wdt_disable();
  clock_prescale_set(clock_div_1);
  DDRB  = 0x00; DDRC  = 0x00; DDRD  = 0x00;   // all inputs...
  PORTB = 0xFF; PORTC = 0xFF; PORTD = 0xFF;  // ...with pull-ups
  USB_Init();
  irInit();
  sei();
  for (;;) {
    usbSendReceive();
    USB_USBTask();
  }
}
