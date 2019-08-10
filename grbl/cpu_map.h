/*
  cpu_map.h - CPU and pin mapping configuration file
  Part of Grbl

  Copyright (c) 2012-2016 Sungeun K. Jeon for Gnea Research LLC

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

/* The cpu_map.h files serve as a central pin mapping selection file for different 
   processor types or alternative pin layouts. This version of Grbl supports only the 
   Arduino Mega2560. */

#ifndef cpu_map_h
#define cpu_map_h

#define PIN_NONE  (*((uint8_t *)0xFFFF))
#define DDR_NONE  (*((uint8_t *)0xFFFF))
#define PORT_NONE (*((uint8_t *)0xFFFF))

#ifdef CPU_MAP_2560_INITIAL // (Arduino Mega 2560) Working @EliteEng

  // Serial port interrupt vectors
  #define SERIAL_RX USART0_RX_vect
  #define SERIAL_UDRE USART0_UDRE_vect

  // Define step pulse output pins. NOTE: All step bit pins must be on the same port.
  #define STEP_DDR      DDRA
  #define STEP_PORT     PORTA
  #define STEP_PIN      PINA
  #define X_STEP_BIT    2 // MEGA2560 Digital Pin 24
  #define Y_STEP_BIT    3 // MEGA2560 Digital Pin 25
  #define Z_STEP_BIT    4 // MEGA2560 Digital Pin 26
  #define STEP_MASK ((1<<X_STEP_BIT)|(1<<Y_STEP_BIT)|(1<<Z_STEP_BIT)) // All step bits

  // Define step direction output pins. NOTE: All direction pins must be on the same port.
  #define DIRECTION_DDR     DDRC
  #define DIRECTION_PORT    PORTC
  #define DIRECTION_PIN     PINC
  #define X_DIRECTION_BIT   7 // MEGA2560 Digital Pin 30
  #define Y_DIRECTION_BIT   6 // MEGA2560 Digital Pin 31
  #define Z_DIRECTION_BIT   5 // MEGA2560 Digital Pin 32
  #define DIRECTION_MASK ((1<<X_DIRECTION_BIT)|(1<<Y_DIRECTION_BIT)|(1<<Z_DIRECTION_BIT)) // All direction bits

  // Define stepper driver enable/disable output pin.
  #define STEPPERS_DISABLE_DDR   DDRB
  #define STEPPERS_DISABLE_PORT  PORTB
  #define STEPPERS_DISABLE_BIT   7 // MEGA2560 Digital Pin 13
  #define STEPPERS_DISABLE_MASK (1<<STEPPERS_DISABLE_BIT)

  // Define homing/hard limit switch input pins and limit interrupt vectors. 
  // NOTE: All limit bit pins must be on the same port
  #define LIMIT_DDR       DDRB
  #define LIMIT_PORT      PORTB
  #define LIMIT_PIN       PINB
  #define X_LIMIT_BIT     4 // MEGA2560 Digital Pin 10
  #define Y_LIMIT_BIT     5 // MEGA2560 Digital Pin 11
  #define Z_LIMIT_BIT     6 // MEGA2560 Digital Pin 12
  #define LIMIT_INT       PCIE0  // Pin change interrupt enable pin
  #define LIMIT_INT_vect  PCINT0_vect 
  #define LIMIT_PCMSK     PCMSK0 // Pin change interrupt register
  #define LIMIT_MASK ((1<<X_LIMIT_BIT)|(1<<Y_LIMIT_BIT)|(1<<Z_LIMIT_BIT)) // All limit bits

  // Define spindle enable and spindle direction output pins.
  #define SPINDLE_ENABLE_DDR      DDRH
  #define SPINDLE_ENABLE_PORT     PORTH
  #define SPINDLE_ENABLE_BIT      3 // MEGA2560 Digital Pin 6
  #define SPINDLE_DIRECTION_DDR   DDRE
  #define SPINDLE_DIRECTION_PORT  PORTE
  #define SPINDLE_DIRECTION_BIT   3 // MEGA2560 Digital Pin 5

  // Define flood and mist coolant enable output pins.
  #define COOLANT_FLOOD_DDR   DDRH
  #define COOLANT_FLOOD_PORT  PORTH
  #define COOLANT_FLOOD_BIT   5 // MEGA2560 Digital Pin 8
  #define COOLANT_MIST_DDR    DDRH
  #define COOLANT_MIST_PORT   PORTH
  #define COOLANT_MIST_BIT    6 // MEGA2560 Digital Pin 9

  // Define user-control CONTROLs (cycle start, reset, feed hold) input pins.
  // NOTE: All CONTROLs pins must be on the same port and not on a port with other input pins (limits).
  #define CONTROL_DDR       DDRK
  #define CONTROL_PIN       PINK
  #define CONTROL_PORT      PORTK
  #define CONTROL_RESET_BIT         0  // MEGA2560 Analog Pin 8
  #define CONTROL_FEED_HOLD_BIT     1  // MEGA2560 Analog Pin 9
  #define CONTROL_CYCLE_START_BIT   2  // MEGA2560 Analog Pin 10
  #define CONTROL_SAFETY_DOOR_BIT   3  // MEGA2560 Analog Pin 11
  #define CONTROL_INT       PCIE2  // Pin change interrupt enable pin
  #define CONTROL_INT_vect  PCINT2_vect
  #define CONTROL_PCMSK     PCMSK2 // Pin change interrupt register
  #define CONTROL_MASK      ((1<<CONTROL_RESET_BIT)|(1<<CONTROL_FEED_HOLD_BIT)|(1<<CONTROL_CYCLE_START_BIT)|(1<<CONTROL_SAFETY_DOOR_BIT))

  // Define probe switch input pin.
  #define PROBE_DDR       DDRK
  #define PROBE_PIN       PINK
  #define PROBE_PORT      PORTK
  #define PROBE_BIT       7  // MEGA2560 Analog Pin 15
  #define PROBE_MASK      (1<<PROBE_BIT)

  // Advanced Configuration Below You should not need to touch these variables
  // Set Timer up to use TIMER4B which is attached to Digital Pin 7
  #define SPINDLE_PWM_MAX_VALUE     1024.0 // Translates to about 1.9 kHz PWM frequency at 1/8 prescaler
  #ifndef SPINDLE_PWM_MIN_VALUE
    #define SPINDLE_PWM_MIN_VALUE   1   // Must be greater than zero.
  #endif
  #define SPINDLE_PWM_OFF_VALUE     0
  #define SPINDLE_PWM_RANGE         (SPINDLE_PWM_MAX_VALUE-SPINDLE_PWM_MIN_VALUE)
  #define SPINDLE_TCCRA_REGISTER		TCCR4A
  #define SPINDLE_TCCRB_REGISTER		TCCR4B
  #define SPINDLE_OCR_REGISTER	  	OCR4B
  #define SPINDLE_COMB_BIT			    COM4B1

  // 1/8 Prescaler, 16-bit Fast PWM mode
  #define SPINDLE_TCCRA_INIT_MASK ((1<<WGM40) | (1<<WGM41))
  #define SPINDLE_TCCRB_INIT_MASK ((1<<WGM42) | (1<<WGM43) | (1<<CS41)) 
  #define SPINDLE_OCRA_REGISTER   OCR4A // 16-bit Fast PWM mode requires top reset value stored here.
  #define SPINDLE_OCRA_TOP_VALUE  0x0400 // PWM counter reset value. Should be the same as PWM_MAX_VALUE in hex.

  // Define spindle output pins.
  #define SPINDLE_PWM_DDR		DDRH
  #define SPINDLE_PWM_PORT  PORTH
  #define SPINDLE_PWM_BIT		4 // MEGA2560 Digital Pin 7

#endif

#ifdef CPU_MAP_2560_RAMPS_BOARD // (Arduino Mega 2560) with Ramps 1.4 Board

  // The GeckoBreakout board is almost identical to a RAMPS in pinouts.
  // The primary difference is that there is a RS485 interface connected to the UART on pins D16 and D17;
  // and the LCD and control connectors have slightly different mappings.
  // #define GeckoBreakout_Variant

  #include "nuts_bolts.h"

  // Serial port interrupt vectors
  #define SERIAL_RX USART0_RX_vect
  #define SERIAL_UDRE USART0_UDRE_vect
  
  // Define ports and pins
  #define DDR(port) DDR##port
  #define _DDR(port) DDR(port)
  #define PORT(port) PORT##port
  #define _PORT(port) PORT(port)
  #define PIN(pin) PIN##pin
  #define _PIN(pin) PIN(pin)
  
  // Define step pulse output pins.  
  #define STEP_PORT_0 F
  #define STEP_PORT_1 F
  #define STEP_PORT_2 L
  #define STEP_BIT_0 0  // X Step - Pin A0
  #define STEP_BIT_1 6  // Y Step - Pin A6
  #define STEP_BIT_2 3  // Z Step - Pin D46
  #define _STEP_BIT(i) STEP_BIT_##i
  #define STEP_BIT(i) _STEP_BIT(i)
  #define STEP_DDR(i) _DDR(STEP_PORT_##i)
  #define _STEP_PORT(i) _PORT(STEP_PORT_##i)
  #define STEP_PORT(i) _STEP_PORT(i)
  #define STEP_PIN(i) _PIN(STEP_PORT_##i)

  // Define Cloned Axis
  //#define CLONE_X_AXIS
  //#define SQUARE_CLONED_X_AXIS
  #define CLONED_X_AXIS_DISABLE_PORT A
  #define CLONED_X_AXIS_DISABLE_BIT 2
  #define CLONED_X_AXIS_STEP_PORT A
  #define CLONED_X_AXIS_STEP_BIT 4
  #define CLONED_X_AXIS_DIRECTION_PORT A
  #define CLONED_X_AXIS_DIRECTION_BIT 6
  
  //#define CLONE_Y_AXIS
  //#define SQUARE_CLONED_Y_AXIS
  #define CLONED_Y_AXIS_DISABLE_PORT C
  #define CLONED_Y_AXIS_DISABLE_BIT 7
  #define CLONED_Y_AXIS_STEP_PORT C
  #define CLONED_Y_AXIS_STEP_BIT 1
  #define CLONED_Y_AXIS_DIRECTION_PORT C
  #define CLONED_Y_AXIS_DIRECTION_BIT 3
  
  #define _CLONED_AXIS_DISABLE_PORT(axis) CLONED_##axis##_AXIS_DISABLE_PORT
  #define CLONED_AXIS_DISABLE_PORT(axis) _PORT(_CLONED_AXIS_DISABLE_PORT(axis))  
  #define CLONED_AXIS_DISABLE_DDR(axis) _DDR(_CLONED_AXIS_DISABLE_PORT(axis))
  #define CLONED_AXIS_DISABLE_BIT(axis) CLONED_##axis##_AXIS_DISABLE_BIT
  #define _CLONED_AXIS_STEP_PORT(axis) CLONED_##axis##_AXIS_STEP_PORT
  #define CLONED_AXIS_STEP_PORT(axis) _PORT(_CLONED_AXIS_STEP_PORT(axis))
  #define CLONED_AXIS_STEP_DDR(axis) _DDR(_CLONED_AXIS_STEP_PORT(axis))
  #define CLONED_AXIS_STEP_BIT(axis) CLONED_##axis##_AXIS_STEP_BIT
  #define _CLONED_AXIS_DIRECTION_PORT(axis) CLONED_##axis##_AXIS_DIRECTION_PORT
  #define CLONED_AXIS_DIRECTION_PORT(axis) _PORT(_CLONED_AXIS_DIRECTION_PORT(axis))
  #define CLONED_AXIS_DIRECTION_DDR(axis) _DDR(_CLONED_AXIS_DIRECTION_PORT(axis))
  #define CLONED_AXIS_DIRECTION_BIT(axis) CLONED_##axis##_AXIS_DIRECTION_BIT
  
  // Define step direction output pins.
  #define DIRECTION_PORT_0 F
  #define DIRECTION_PORT_1 F
  #define DIRECTION_PORT_2 L
  #define DIRECTION_BIT_0 1 // X Dir - Pin A1
  #define DIRECTION_BIT_1 7 // Y Dir - Pin A7
  #define DIRECTION_BIT_2 1 // Z Dir - Pin D48
  #define _DIRECTION_BIT(i) DIRECTION_BIT_##i
  #define DIRECTION_BIT(i) _DIRECTION_BIT(i)
  #define DIRECTION_DDR(i) _DDR(DIRECTION_PORT_##i)
  #define _DIRECTION_PORT(i) _PORT(DIRECTION_PORT_##i)
  #define DIRECTION_PORT(i) _DIRECTION_PORT(i)
  #define DIRECTION_PIN(i) _PIN(DIRECTION_PORT_##i)

  // Define stepper driver enable/disable output pin.
  #define STEPPER_DISABLE_PORT_0 D
  #define STEPPER_DISABLE_PORT_1 F
  #define STEPPER_DISABLE_PORT_2 K
  #define STEPPER_DISABLE_BIT_0 7 // X Enable - Pin D38
  #define STEPPER_DISABLE_BIT_1 2 // Y Enable - Pin A2
  #define STEPPER_DISABLE_BIT_2 0 // Z Enable - Pin A8
  #define STEPPER_DISABLE_BIT(i) STEPPER_DISABLE_BIT_##i
  #define STEPPER_DISABLE_DDR(i) _DDR(STEPPER_DISABLE_PORT_##i)
  #define STEPPER_DISABLE_PORT(i) _PORT(STEPPER_DISABLE_PORT_##i)
  #define STEPPER_DISABLE_PIN(i) _PIN(STEPPER_DISABLE_PORT_##i)

  // Define homing/hard limit switch input pins and limit interrupt vectors. 
  #ifndef USE_UI_ENCODER_B_ISR
  
    #define MIN_LIMIT_PORT_0 E
    #define MIN_LIMIT_PORT_1 J
    #define MIN_LIMIT_PORT_2 D
    #define MIN_LIMIT_BIT_0 5 // X Limit Min - Pin D3
    #define MIN_LIMIT_BIT_1 1 // Y Limit Min - Pin D14
    #define MIN_LIMIT_BIT_2 3 // Z Limit Min - Pin D18
    #define _MIN_LIMIT_BIT(i) MIN_LIMIT_BIT_##i
    #define MIN_LIMIT_BIT(i) _MIN_LIMIT_BIT(i)
    #define MIN_LIMIT_DDR(i) _DDR(MIN_LIMIT_PORT_##i)
    #define MIN_LIMIT_PORT(i) _PORT(MIN_LIMIT_PORT_##i)
    #define MIN_LIMIT_PIN(i) _PIN(MIN_LIMIT_PORT_##i)

    #define MAX_LIMIT_PORT_0 E
    #define MAX_LIMIT_PORT_1 J
    #define MAX_LIMIT_PORT_2 D
    #define MAX_LIMIT_BIT_0 4 // X Limit Max - Pin D2
    #define MAX_LIMIT_BIT_1 0 // Y Limit Max - Pin D15
    #define MAX_LIMIT_BIT_2 2 // Z Limit Max - Pin D19
    #define _MAX_LIMIT_BIT(i) MAX_LIMIT_BIT_##i
    #define MAX_LIMIT_BIT(i) _MAX_LIMIT_BIT(i)
    #define MAX_LIMIT_DDR(i) _DDR(MAX_LIMIT_PORT_##i)
    #define MAX_LIMIT_PORT(i) _PORT(MAX_LIMIT_PORT_##i)
    #define MAX_LIMIT_PIN(i) _PIN(MAX_LIMIT_PORT_##i)

  #else
    
    // In RAMPS, the pins marked for X min and max limit inputs are needed for
    // UI Encoder B because they have interrupt capability. So, the inputs are
    // re-assigned as follows:
    // RAMPS Label:       GRBL Function:
    //  Y-                X Minimum Limit Switch
    //  Y                 Y Minimum Limit Switch
    //  Z-                Unused (unless Axis Cloning is enabled)
    //  Z                 Z Maximum Limit Switch
    
    #define MIN_LIMIT_PORT_0 J
    #define MIN_LIMIT_PORT_1 J
    #define MIN_LIMIT_PORT_2 _NONE
    #define MIN_LIMIT_BIT_0 1 // X Limit Min - Pin D14
    #define MIN_LIMIT_BIT_1 0 // Y Limit Min - Pin D15
    #define MIN_LIMIT_BIT_2 0 // Z Limit Min - Unused
    #define _MIN_LIMIT_BIT(i) MIN_LIMIT_BIT_##i
    #define MIN_LIMIT_BIT(i) _MIN_LIMIT_BIT(i)
    #define MIN_LIMIT_DDR(i) _DDR(MIN_LIMIT_PORT_##i)
    #define MIN_LIMIT_PORT(i) _PORT(MIN_LIMIT_PORT_##i)
    #define MIN_LIMIT_PIN(i) _PIN(MIN_LIMIT_PORT_##i)

    #define MAX_LIMIT_PORT_0 _NONE
    #define MAX_LIMIT_PORT_1 _NONE
    #define MAX_LIMIT_PORT_2 D
    #define MAX_LIMIT_BIT_0 0 // X Limit Max - Unused
    #define MAX_LIMIT_BIT_1 0 // Y Limit Max - Unused
    #define MAX_LIMIT_BIT_2 2 // Z Limit Max - Pin D19
    #define _MAX_LIMIT_BIT(i) MAX_LIMIT_BIT_##i
    #define MAX_LIMIT_BIT(i) _MAX_LIMIT_BIT(i)
    #define MAX_LIMIT_DDR(i) _DDR(MAX_LIMIT_PORT_##i)
    #define MAX_LIMIT_PORT(i) _PORT(MAX_LIMIT_PORT_##i)
    #define MAX_LIMIT_PIN(i) _PIN(MAX_LIMIT_PORT_##i)

  #endif

  //  #define LIMIT_INT       PCIE0  // Pin change interrupt enable pin
  //  #define LIMIT_INT_vect  PCINT0_vect 
  //  #define LIMIT_PCMSK     PCMSK0 // Pin change interrupt register
  //  #define LIMIT_MASK ((1<<X_LIMIT_BIT)|(1<<Y_LIMIT_BIT)|(1<<Z_LIMIT_BIT)) // All limit bits
  #define DISABLE_HW_LIMITS

  // Define spindle enable and spindle direction output pins.
  #define SPINDLE_ENABLE_DDR      DDRG
  #define SPINDLE_ENABLE_PORT     PORTG
  #define SPINDLE_ENABLE_BIT      5 // MEGA2560 Digital Pin 4 - Ramps 1.4 Servo 4 Signal pin
  #define SPINDLE_DIRECTION_DDR   DDRE
  #define SPINDLE_DIRECTION_PORT  PORTE
  #define SPINDLE_DIRECTION_BIT   3 // MEGA2560 Digital Pin 5 - Ramps 1.4 Servo 3 Signal pin

  // Define flood and mist coolant enable output pins.
  #define COOLANT_FLOOD_DDR   DDRB
  #define COOLANT_FLOOD_PORT  PORTB
  #define COOLANT_FLOOD_BIT   4 // MEGA2560 Digital Pin 10 - Ramps 1.4 12v output
  #define COOLANT_MIST_DDR    DDRH
  #define COOLANT_MIST_PORT   PORTH
  #define COOLANT_MIST_BIT    6 // MEGA2560 Digital Pin 9 - Ramps 1.4 12v output

  #ifndef USE_UI_SUPPORT
  
    // Define user-control CONTROLs (cycle start, reset, feed hold) input pins.
    // NOTE: All CONTROLs pins must be on the same port and not on a port with other input pins (limits).
    #define CONTROL_DDR       DDRK
    #define CONTROL_PIN       PINK
    #define CONTROL_PORT      PORTK
    #define CONTROL_RESET_BIT         1  // Pin A9 - RAMPS Aux 2 Port
    #define CONTROL_FEED_HOLD_BIT     2  // Pin A10 - RAMPS Aux 2 Port
    #define CONTROL_CYCLE_START_BIT   3  // Pin A11 - RAMPS Aux 2 Port
    #define CONTROL_SAFETY_DOOR_BIT   4  // Pin A12 - RAMPS Aux 2 Port
    #define CONTROL_INT       PCIE2  // Pin change interrupt enable pin
    #define CONTROL_INT_vect  PCINT2_vect
    #define CONTROL_PCMSK     PCMSK2 // Pin change interrupt register
    #define CONTROL_MASK      ((1<<CONTROL_RESET_BIT)|(1<<CONTROL_FEED_HOLD_BIT)|(1<<CONTROL_CYCLE_START_BIT)|(1<<CONTROL_SAFETY_DOOR_BIT))
    #define USE_CONTROL_ISR
    
  #else // USE_UI_SUPPORT
  
    // UI Support requires a matrix keypad, which is by default setup on the Aux 2 port of a RAMPS board.
    // Consequentally, the control inputs need to be moved to other pins... by default, unused pins on the Aux 4 header of the RAMPS board.
    // Also, these pins don't have an interrupt associated with them, so they are monitored in a polling loop by UITask().
    #define CONTROL_DDR       DDRL
    #define CONTROL_PIN       PINL
    #define CONTROL_PORT      PORTL
    #ifndef GeckoBreakout_Variant
      #define CONTROL_RESET_BIT         2  // Pin D47 - RAMPS Aux 4 Port
    #endif
    #define CONTROL_FEED_HOLD_BIT     4  // Pin D45 - RAMPS Aux 4 Port
    #define CONTROL_SAFETY_DOOR_BIT   6  // Pin D43 - RAMPS Aux 4 Port
    #ifndef GeckoBreakout_Variant
      #define CONTROL_MASK      ((1<<CONTROL_RESET_BIT)|(1<<CONTROL_FEED_HOLD_BIT)|(1<<CONTROL_SAFETY_DOOR_BIT))
    #else
      #define CONTROL_MASK      ((1<<CONTROL_FEED_HOLD_BIT)|(1<<CONTROL_SAFETY_DOOR_BIT))
    #endif

    #ifndef GeckoBreakout_Variant
      #define CONTROL_CYCLE_START_DDR       DDRC
      #define CONTROL_CYCLE_START_PIN       PINC
      #define CONTROL_CYCLE_START_PORT      PORTC
      #define CONTROL_CYCLE_START_BIT       5 // Pin D32 - RAMPS Aux 4 Port
    #else
      #define CONTROL_CYCLE_START_DDR       DDRL
      #define CONTROL_CYCLE_START_PIN       PINL
      #define CONTROL_CYCLE_START_PORT      PORTL
      #define CONTROL_CYCLE_START_BIT       2 // Pin D47 - GeckoBreakout
    #endif

  #endif // USE_UI_SUPPORT

  // Define probe switch input pin.
  #define PROBE_DDR       DDRF
  #define PROBE_PIN       PINF
  #define PROBE_PORT      PORTF
  #define PROBE_BIT       3  // MEGA2560 Digital Pin 57; on RAMPS Aux 1
  #define PROBE_MASK      (1<<PROBE_BIT)

  // Advanced Configuration Below -- You should not need to touch these variables
  // Set Timer up to use TIMER4A which is attached to Digital Pin 6
  #define SPINDLE_PWM_MAX_VALUE     1023.0 // Translates to about 1.9 kHz PWM frequency at 1/8 prescaler
  #ifndef SPINDLE_PWM_MIN_VALUE
  #define SPINDLE_PWM_MIN_VALUE     1   // Must be greater than zero.
  #endif
  #define SPINDLE_PWM_OFF_VALUE     0
  #define SPINDLE_PWM_RANGE         (SPINDLE_PWM_MAX_VALUE-SPINDLE_PWM_MIN_VALUE)

  //Control Digital Pin 6 which is Servo 2 signal pin on Ramps 1.4 board
  #define SPINDLE_TCCRA_REGISTER    TCCR4A
  #define SPINDLE_TCCRB_REGISTER    TCCR4B
  #define SPINDLE_OCR_REGISTER      OCR4A
  #define SPINDLE_COMB_BIT          COM4A1

  // 1/8 Prescaler, 10-bit Fast PWM mode
  #define SPINDLE_TCCRA_INIT_MASK ((1<<WGM40) | (1<<WGM41))
  #define SPINDLE_TCCRB_INIT_MASK ((1<<WGM42) | (1<<CS41)) 

  // Define spindle output pins.
  #define SPINDLE_PWM_DDR   DDRH
  #define SPINDLE_PWM_PORT  PORTH
  #define SPINDLE_PWM_BIT   3 // MEGA2560 Digital Pin 6 

  // If enabled, define the pin configuration for the SD interface.
  #ifdef USE_SD_SUPPORT
    #define SD_CHIPSELECT 53
    #define SD_CARDDETECT 49 //[RAMPS14-SMART-ADAPTER]
  #endif
  
  #ifdef USE_UI_SUPPORT
  
    #define BEEPER 37 //[RAMPS14-SMART-ADAPTER]
  
  // Define the pin configuration for the LCD interface.
    #ifndef GeckoBreakout_Variant
      #define LCD_PIN_RS 16 //[RAMPS14-SMART-ADAPTER]
      #define LCD_PIN_EN 17 //[RAMPS14-SMART-ADAPTER]
    #else
      #define LCD_PIN_RS 39 //[GeckoBreakout_Variant]
      #define LCD_PIN_EN 32 //[GeckoBreakout_Variant]
    #endif
    
    #define LCD_PIN_D4 23 //[RAMPS14-SMART-ADAPTER]
    #define LCD_PIN_D5 25 //[RAMPS14-SMART-ADAPTER]
    #define LCD_PIN_D6 27 //[RAMPS14-SMART-ADAPTER]
    #define LCD_PIN_D7 29 //[RAMPS14-SMART-ADAPTER]
    
    // The command execution time for the LCD, in microseconds.
    // If your LCD is not working, try increasing this value.
    #define LCD_SEND_DELAY  50
        
  // Define the pin configuration for the UI Rotary Encoder interface.
    #define UI_ENCODER_A_PINBTN1 35 //[RAMPS14-SMART-ADAPTER]
    #define UI_ENCODER_A_PINBTN2 41 //[RAMPS14-SMART-ADAPTER]
    #define UI_ENCODER_A_PINBTN1_ENABLE_PULLUP
    #define UI_ENCODER_A_PINBTN2_ENABLE_PULLUP
    #define UI_ENCODER_A_PINBTN1_ACTIVE_LOW
    #define UI_ENCODER_A_PINBTN2_ACTIVE_LOW
    
    
    //encoder pins
    // Encoder modes:
    // 1 = four states per detent, normally high.
    // 2 = four states per detent, normally low.
    // 3 = one state per detent.
     
    // Typically, UI Encoder A is a mechanical encoder with 4 states per detent; normally high.
    #define UI_ENCODER_A_MODE 1
    #define UI_ENCODER_A_PIN1 33 //[RAMPS14-SMART-ADAPTER]
    #define UI_ENCODER_A_PIN2 31 //[RAMPS14-SMART-ADAPTER]
    #define UI_ENCODER_A_PIN1_ENABLE_PULLUP
    #define UI_ENCODER_A_PIN2_ENABLE_PULLUP
    
    #define UI_ENCODER_B_MODE 2
    #define UI_ENCODER_B_PIN1 2 //[RAMPS 1.4]
    #define UI_ENCODER_B_PIN2 3 //[RAMPS 1.4]
    // Typically, encoder B is used with a TTL MPG handwheel... which should not have the pullups enabled.
    //#define UI_ENCODER_B_PIN1_ENABLE_PULLUP
    //#define UI_ENCODER_B_PIN2_ENABLE_PULLUP
    
  // Define the pin configuration for the Matrix Keypad interface.
    #define UI_MATRIX_KEYPAD_PIN_R1 65
    #define UI_MATRIX_KEYPAD_PIN_R2 42
    #define UI_MATRIX_KEYPAD_PIN_R3 40
    #define UI_MATRIX_KEYPAD_PIN_R4 63
    #define UI_MATRIX_KEYPAD_PIN_C1 59
    #define UI_MATRIX_KEYPAD_PIN_C2 64
    #define UI_MATRIX_KEYPAD_PIN_C3 44
    #define UI_MATRIX_KEYPAD_PIN_C4 66
    
  #endif // USE_UI_SUPPORT
  
#endif
/* 
#ifdef CPU_MAP_CUSTOM_PROC
  // For a custom pin map or different processor, copy and edit one of the available cpu
  // map files and modify it to your needs. Make sure the defined name is also changed in
  // the config.h file.
#endif
*/

#endif
