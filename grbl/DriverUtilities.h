// Code taken from Arduino. Licensed under LGPL 2.1.

#ifndef DRIVER_UTILITIES_H
#define DRIVER_UTILITIES_H

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>

#include <avr/pgmspace.h>
#include <avr/io.h>
#include <avr/interrupt.h>

typedef bool boolean;
typedef uint8_t byte;

#define lowByte(w) ((uint8_t) ((w) & 0xff))
#define highByte(w) ((uint8_t) ((w) >> 8))

#define bitRead(value, bit) (((value) >> (bit)) & 0x01)
#define bitSet(value, bit) ((value) |= (1UL << (bit)))
#define bitClear(value, bit) ((value) &= ~(1UL << (bit)))
#define bitWrite(value, bit, bitvalue) (bitvalue ? bitSet(value, bit) : bitClear(value, bit))


#include "binary_grbl.h"
#include "pins_arduino_mega.h"


#ifdef __cplusplus
  extern "C"{
#endif

  #ifndef serial_h
    #include "serial.h"
  #endif

  #ifndef print_h
    #include "serialprint.h"
  #endif

  void clock_init();
  extern volatile uint32_t clock_ticks_counter; // Always access this with interrupts disabled! Increments at 976hz from Timer 2.
  static inline __attribute__((always_inline)) uint32_t clock_ticks()
  {
    uint8_t oldSREG = SREG;
    cli();
      uint32_t value = clock_ticks_counter;
    SREG = oldSREG;
    return value;
  }
  
  void delay_clock_ticks(uint32_t ticks);
  void blocking_delay_clock_ticks(uint32_t ticks);  // Warning: this delay does NOT call protocol_execute_realtime()!
  void delayMicros(unsigned int us);

  float stringToFloat(char *string);
  char *sPrintFloat(char *result, uint8_t fieldWidth, float n, uint8_t decimal_places);
  void sPrintFloatRightJustified(char *result, uint8_t fieldWidth, float n, uint8_t decimal_places);




#ifdef __cplusplus
  } // extern "C"
#endif

#endif