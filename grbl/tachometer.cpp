/*
 * Copyright (c) 2019 by Tim hawkins <tim.thawkins@gmail.com>
 *
 * Tachometer support for grbl-Mega.
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of either the GNU General Public License version 2
 * or the GNU Lesser General Public License version 2.1, both as
 * published by the Free Software Foundation.
 */

#include "grbl.h"
#include "PinChangeInt.h"

static unsigned int tachometer_count = 0; 
static unsigned int tachometer_poll_count = 0; 

const int TACHPULSE_PIN = 2;

void setup_tachometer(void){
    tachometer_count = 0;
    tachometer_poll_count = 0;
    setPinMode(TACHPULSE_PIN, INPUT_PULLUP);
    attachPinChangeInterrupt(TACHPULSE_PIN, isr_tachometer, FALLING);
}

unsigned int poll_tachometer(void){
      tachometer_poll_count=0;
      unsigned int save_count =  tachometer_count;
      tachometer_count = 0;
      return save_count;
}

void isr_tachometer(void){
      tachometer_count++;
}