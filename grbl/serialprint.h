/*
  print.h - Functions for formatting output strings
  Part of Grbl

  Copyright (c) 2011-2016 Sungeun K. Jeon for Gnea Research LLC
  Copyright (c) 2009-2011 Simen Svale Skogsrud

  Grbl is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Grbl is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Grbl.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef print_h
#define print_h

#ifdef __cplusplus
  extern "C"{
#endif

  #include <stdint.h>

  extern void printString(const char *s);
  extern void printlnString(const char *s);

  extern void printPgmString(const char *s);

  extern void printInteger(long n);
  extern void printlnInteger(long n);

  extern void print_uint32_base10(uint32_t n);
  extern void println_uint32_base10(uint32_t n);

  // Prints an uint8 variable in base 10.
  extern void print_uint8_base10(uint8_t n);

  // Prints an uint8 variable in base 2 with desired number of desired digits.
  extern void print_uint8_base2_ndigit(uint8_t n, uint8_t digits);

  extern void printFloat(float n, uint8_t decimal_places);
  extern void printlnFloat(float n, uint8_t decimal_places);

  // Floating value printing handlers for special variables types used in Grbl.
  //  - CoordValue: Handles all position or coordinate values in inches or mm reporting.
  //  - RateValue: Handles feed rate and current velocity in inches or mm reporting.
  extern void printFloat_CoordValue(float n);
  extern void printFloat_RateValue(float n);

  // Debug tool to print free memory in bytes at the called point. Not used otherwise.
  extern void printFreeMemory();

#ifdef __cplusplus
  } // extern "C"
#endif

#endif
