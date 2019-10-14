// Code taken from Arduino. Licensed under LGPL 2.1.

#include "DriverUtilities.h"

#include "protocol.h"

// *********************************************************************
// ~1Khz Timer using AVR Timer 2

volatile uint32_t clock_ticks_counter;

void clock_init()
{
	uint8_t oldSREG = SREG;
	cli();

    clock_ticks_counter = 0;

    TCCR2A = 0x00;
    TCCR2B = 0x04; // Div 64 Prescaler. Normal mode. 976hz clock tick frequency on 16Mhz system clock.
    ASSR = 0x00;
    TCNT2 = 0x00;
    TIMSK2 = 0x01; // Interrupt on overflow.
  
	SREG = oldSREG;
}

ISR(TIMER2_OVF_vect) { clock_ticks_counter++; }

void delay_clock_ticks(uint32_t ticks)
{
  uint32_t end_ticks = clock_ticks() + ticks;
  while(clock_ticks() <= end_ticks) protocol_execute_realtime();
}

void blocking_delay_clock_ticks(uint32_t ticks)
{
  uint32_t end_ticks = clock_ticks() + ticks;
  while(clock_ticks() <= end_ticks);
}


// *********************************************************************
// Delay for the given number of microseconds.  Assumes a 1, 8, 12, 16, 20 or 24 MHz clock.

void delayMicros(unsigned int us)
{
	// call = 4 cycles + 2 to 4 cycles to init us(2 for constant delay, 4 for variable)

	// calling avrlib's delay_us() function with low values (e.g. 1 or
	// 2 microseconds) gives delays longer than desired.
	//delay_us(us);
#if F_CPU >= 24000000L
	// for the 24 MHz clock for the aventurous ones, trying to overclock

	// zero delay fix
	if (!us) return; //  = 3 cycles, (4 when true)

	// the following loop takes a 1/6 of a microsecond (4 cycles)
	// per iteration, so execute it six times for each microsecond of
	// delay requested.
	us *= 6; // x6 us, = 7 cycles

	// account for the time taken in the preceeding commands.
	// we just burned 22 (24) cycles above, remove 5, (5*4=20)
	// us is at least 6 so we can substract 5
	us -= 5; //=2 cycles

#elif F_CPU >= 20000000L
	// for the 20 MHz clock on rare Arduino boards

	// for a one-microsecond delay, simply return.  the overhead
	// of the function call takes 18 (20) cycles, which is 1us
	__asm__ __volatile__ (
		"nop" "\n\t"
		"nop" "\n\t"
		"nop" "\n\t"
		"nop"); //just waiting 4 cycles
	if (us <= 1) return; //  = 3 cycles, (4 when true)

	// the following loop takes a 1/5 of a microsecond (4 cycles)
	// per iteration, so execute it five times for each microsecond of
	// delay requested.
	us = (us << 2) + us; // x5 us, = 7 cycles

	// account for the time taken in the preceeding commands.
	// we just burned 26 (28) cycles above, remove 7, (7*4=28)
	// us is at least 10 so we can substract 7
	us -= 7; // 2 cycles

#elif F_CPU >= 16000000L
	// for the 16 MHz clock on most Arduino boards

	// for a one-microsecond delay, simply return.  the overhead
	// of the function call takes 14 (16) cycles, which is 1us
	if (us <= 1) return; //  = 3 cycles, (4 when true)

	// the following loop takes 1/4 of a microsecond (4 cycles)
	// per iteration, so execute it four times for each microsecond of
	// delay requested.
	us <<= 2; // x4 us, = 4 cycles

	// account for the time taken in the preceeding commands.
	// we just burned 19 (21) cycles above, remove 5, (5*4=20)
	// us is at least 8 so we can substract 5
	us -= 5; // = 2 cycles,

#elif F_CPU >= 12000000L
	// for the 12 MHz clock if somebody is working with USB

	// for a 1 microsecond delay, simply return.  the overhead
	// of the function call takes 14 (16) cycles, which is 1.5us
	if (us <= 1) return; //  = 3 cycles, (4 when true)

	// the following loop takes 1/3 of a microsecond (4 cycles)
	// per iteration, so execute it three times for each microsecond of
	// delay requested.
	us = (us << 1) + us; // x3 us, = 5 cycles

	// account for the time taken in the preceeding commands.
	// we just burned 20 (22) cycles above, remove 5, (5*4=20)
	// us is at least 6 so we can substract 5
	us -= 5; //2 cycles

#elif F_CPU >= 8000000L
	// for the 8 MHz internal clock

	// for a 1 and 2 microsecond delay, simply return.  the overhead
	// of the function call takes 14 (16) cycles, which is 2us
	if (us <= 2) return; //  = 3 cycles, (4 when true)

	// the following loop takes 1/2 of a microsecond (4 cycles)
	// per iteration, so execute it twice for each microsecond of
	// delay requested.
	us <<= 1; //x2 us, = 2 cycles

	// account for the time taken in the preceeding commands.
	// we just burned 17 (19) cycles above, remove 4, (4*4=16)
	// us is at least 6 so we can substract 4
	us -= 4; // = 2 cycles

#else
	// for the 1 MHz internal clock (default settings for common Atmega microcontrollers)

	// the overhead of the function calls is 14 (16) cycles
	if (us <= 16) return; //= 3 cycles, (4 when true)
	if (us <= 25) return; //= 3 cycles, (4 when true), (must be at least 25 if we want to substract 22)

	// compensate for the time taken by the preceeding and next commands (about 22 cycles)
	us -= 22; // = 2 cycles
	// the following loop takes 4 microseconds (4 cycles)
	// per iteration, so execute it us/4 times
	// us is at least 4, divided by 4 gives us 1 (no zero delay bug)
	us >>= 2; // us div 4, = 4 cycles
	

#endif

	// busy wait
	__asm__ __volatile__ (
		"1: sbiw %0,1" "\n\t" // 2 cycles
		"brne 1b" : "=w" (us) : "0" (us) // 2 cycles
	);
	// return = 4 cycles
}

#define MAX_INT_DIGITS 8 // Maximum number of digits in int32 (and float)

// Extracts a floating point value from a string. The following code is based loosely on
// the avr-libc strtod() function by Michael Stumpf and Dmitry Xmelkov and many freely
// available conversion method examples, but has been highly optimized for Grbl. For known
// CNC applications, the typical decimal value is expected to be in the range of E0 to E-4.
// Scientific notation is officially not supported by g-code, and the 'E' character may
// be a g-code word on some CNC systems. So, 'E' notation will not be recognized.
// NOTE: Thanks to Radu-Eosif Mihailescu for identifying the issues with using strtod().
float stringToFloat(char *string)
{
  unsigned char c;

  // Grab first character and increment pointer. No spaces assumed in line.
  c = *string++;

  // Capture initial positive/minus character
  bool isnegative = false;
  if (c == '-') {
    isnegative = true;
    c = *string++;
  } else if (c == '+') {
    c = *string++;
  }

  // Extract number into fast integer. Track decimal in terms of exponent value.
  uint32_t intval = 0;
  int8_t exp = 0;
  uint8_t ndigit = 0;
  bool isdecimal = false;
  for(;;) {
    c -= '0';
    if (c <= 9) {
      ndigit++;
      if (ndigit <= MAX_INT_DIGITS) {
        if (isdecimal) { exp--; }
        intval = (((intval << 2) + intval) << 1) + c; // intval*10 + c
      } else {
        if (!(isdecimal)) { exp++; }  // Drop overflow digits
      }
    } else if (c == (('.'-'0') & 0xff)  &&  !(isdecimal)) {
      isdecimal = true;
    } else {
      break;
    }
    c = *string++;
  }

  // Return if no digits have been read.
  if (!ndigit) { return 0.0; };

  // Convert integer into floating point.
  float fval;
  fval = (float)intval;

  // Apply decimal. Should perform no more than two floating point multiplications for the
  // expected range of E0 to E-4.
  if (fval != 0) {
    while (exp <= -2) {
      fval *= 0.01;
      exp += 2;
    }
    if (exp < 0) {
      fval *= 0.1;
    } else if (exp > 0) {
      do {
        fval *= 10.0;
      } while (--exp > 0);
    }
  }

  // Assign floating point value with correct sign.
  if (isnegative) {
    return -fval;
  } else {
    return fval;
  }
}

// Convert float to string by immediately converting to a long integer, which contains
// more digits than a float. Number of decimal places, which are tracked by a counter,
// may be set by the user. The integer is then efficiently converted to a string.
// Returns a pointer to the character immediately after the last character written.
// NOTE: AVR '%' and '/' integer operations are very efficient. Bitshifting speed-up
// techniques are actually just slightly slower. Found this out the hard way.
char *sPrintFloat(char *result, uint8_t fieldWidth, float n, uint8_t decimal_places)
{
  char buffer[13];
  uint8_t i = 0;
  bool wasNegative;
  if (n < 0)
  {
    wasNegative = true;
    n = -n;
  }
  else
  {
    wasNegative = false;
  }

  uint8_t decimals = decimal_places;
  while (decimals >= 2)
  { // Quickly convert values expected to be E0 to E-4.
    n *= 100;
    decimals -= 2;
  }
  if (decimals) n *= 10;
  n += 0.5; // Add rounding factor. Ensures carryover through entire value.

  // Generate digits backwards and store in string.
  uint32_t a = (uint32_t)n;
  while(a > 0)
  {
    buffer[i++] = (a % 10) + '0'; // Get digit
    a /= 10;
  }
  while (i < decimal_places)
     buffer[i++] = '0'; // Fill in zeros to decimal point for (n < 1)
  if (i == decimal_places) // Fill in leading zero, if needed.
    buffer[i++] = '0';

  // Print the generated string.
  if(wasNegative)
  {
    *result++ = '-';
    fieldWidth--;
  }
  for (; i && fieldWidth; i--)
  {
    if (i == decimal_places) // Insert decimal point in right place.
    {
      *result++ = '.';
      fieldWidth--;
    }
    *result++ = buffer[i-1];
    fieldWidth--;
  }
  return result;
}

// Similar to sPrintFloat() except the text is placed at the end of the specified buffer
// and fieldWidth is used to pad any excess space in result with ' ' characters.
void sPrintFloatRightJustified(char *result, uint8_t fieldWidth, float n, uint8_t decimal_places)
{
  char buffer[13];
  uint8_t i = 0;
  bool wasNegative;
  if (n < 0)
  {
    wasNegative = true;
    n = -n;
  }
  else
  {
    wasNegative = false;
  }

  uint8_t decimals = decimal_places;
  while (decimals >= 2)
  { // Quickly convert values expected to be E0 to E-4.
    n *= 100;
    decimals -= 2;
  }
  if (decimals) n *= 10;
  n += 0.5; // Add rounding factor. Ensures carryover through entire value.

  // Generate digits backwards and store in string.
  uint32_t a = (uint32_t)n;
  while(a > 0)
  {
    buffer[i++] = (a % 10) + '0'; // Get digit
    a /= 10;
  }
  while (i < decimal_places)
     buffer[i++] = '0'; // Fill in zeros to decimal point for (n < 1)
  if (i == decimal_places) // Fill in leading zero, if needed.
    buffer[i++] = '0';

  // Print the generated string.
  uint8_t totalWidth = i;
  if(wasNegative) totalWidth++;
  if(decimal_places) totalWidth++;
  if(fieldWidth > totalWidth)
  {
    uint8_t padding = fieldWidth - totalWidth;
    for(; padding; padding--) *result++ = ' ';
  }
  
  if(wasNegative)
  {
    *result++ = '-';
    fieldWidth--;
  }
  for (; i && fieldWidth; i--)
  {
    if (i == decimal_places) // Insert decimal point in right place.
    {
      *result++ = '.';
      fieldWidth--;
    }
    *result++ = buffer[i-1];
    fieldWidth--;
  }
}

