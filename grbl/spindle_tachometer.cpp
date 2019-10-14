#include "grbl.h"

#ifdef USE_SPINDLE_TACHOMETER

  #if( !defined(SPINDLE_TACHOMETER_RISING_EDGE) && !defined(SPINDLE_TACHOMETER_FALLING_EDGE) )
    #error "The spindle tachometer requires SPINDLE_TACHOMETER_RISING_EDGE, SPINDLE_TACHOMETER_FALLING_EDGE, or both to be defined."
  #endif
  
  #define SPINDLE_TACHOMETER_SEC_PER_TICK (0.000016)

  static volatile uint16_t spindle_tachometer_ticks;
  static volatile bool spindle_tachometer_did_saturate;

  void spindle_tachometer_init()
  {
    PCICR |= (1 << PCIE0);
    PCMSK0 |= (1 << PCINT5);
  
    // Normal mode; 15625 hz; interrupt on overflow
    TCCR5A = 0;
    TCCR5B = (1 << CS52);// | (1 << CS50);
    TCNT5  = 0;
    TIMSK5 |= (1 << TOIE5);
  }

  ISR(PCINT0_vect)
  {
    #if( defined(SPINDLE_TACHOMETER_RISING_EDGE) && !defined(SPINDLE_TACHOMETER_FALLING_EDGE) )
      if(!fastDigitalRead(11)) return;
    #endif
  
    #if( !defined(SPINDLE_TACHOMETER_RISING_EDGE) && defined(SPINDLE_TACHOMETER_FALLING_EDGE) )
      if(fastDigitalRead(11)) return;
    #endif

    if(!spindle_tachometer_did_saturate)
    {
      spindle_tachometer_ticks = TCNT5;
      TCNT5 = 0;
    }
    spindle_tachometer_did_saturate=false;
  }

  ISR(TIMER5_OVF_vect)
  {
    spindle_tachometer_ticks = 0xFFFF;
    spindle_tachometer_did_saturate=true;
  }

  float spindle_tachometer_calculate_RPM()
  {
    uint8_t oldSREG = SREG;
    cli();
      bool spindle_tachometer_did_saturate_value = spindle_tachometer_did_saturate;
      uint16_t spindle_tachometer_ticks_value = spindle_tachometer_ticks;
    SREG = oldSREG;
    if(spindle_tachometer_did_saturate_value) return 0.0;
    return 60.0/(spindle_tachometer_ticks_value * SPINDLE_TACHOMETER_SEC_PER_TICK);
  }

#endif