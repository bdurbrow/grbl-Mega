/*
  limitswitches.h - code pertaining to limit-switches and performing the homing cycle
  Part of Grbl

  Copyright (c) 2012-2016 Sungeun K. Jeon for Gnea Research LLC
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

#ifndef limitswitches_h
#define limitswitches_h

// Initialize the limits module
void limits_init();

// Disables hard limits.
void limits_disable();

// Returns limit state as a bit-wise uint8 variable.
uint8_t limits_get_state();

// Perform one portion of the homing cycle based on the input settings.
void limits_go_home(uint8_t cycle_mask);

#if(defined(SQUARE_CLONED_X_AXIS) || defined(SQUARE_CLONED_Y_AXIS))
  #define LIMITS_AUTOSQUARING_OK              0
  #define LIMITS_AUTOSQUARING_ERROR_IN_ABORT  1
  #define LIMITS_AUTOSQUARING_ERROR_NOT_IDLE  2
  #define LIMITS_AUTOSQUARING_ERROR_NOT_HOME  3
  #define LIMITS_AUTOSQUARING_ERROR_CANCELED  4
  #define LIMITS_AUTOSQUARING_ERROR_FAILED    5

  // Perform axis auto-squaring operation. Machine must be not in an abort state; idle; and at home position on entry.
  uint8_t limits_square_axis();
#endif

// Check for soft limit violations
void limits_soft_check(float *target);

#endif
