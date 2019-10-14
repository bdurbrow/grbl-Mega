#ifndef spindle_tachometer_h
#define spindle_tachometer_h
  #ifdef USE_SPINDLE_TACHOMETER
    extern void spindle_tachometer_init();
    extern float spindle_tachometer_calculate_RPM();
  #endif
#endif