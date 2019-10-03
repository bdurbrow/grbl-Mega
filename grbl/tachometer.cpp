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
static unsigned int tachometer_rpm = 0; 
static unsigned int tachometer_rpm_array[5] = {0,0,0,0,0};

const int TACHPULSE_PIN = 11;

void setup_tachometer(void){
    tachometer_count = 0;
    tachometer_poll_count = 0;
    setPinMode(TACHPULSE_PIN, INPUT_PULLUP);
    attachPinChangeInterrupt(TACHPULSE_PIN, pin_itr_tachometer, FALLING);

 //set timer5 interrupt at 8kHz
  TCCR5A = 0;// set entire TCCR5A register to 0
  TCCR5B = 0;// same for TCCR5B
  TCNT5  = 0;//initialize counter value to 0
  // set compare match register for 8khz increments
  OCR5A = 249;// = (16*10^6) / (8000*8) - 1 (must be <256)
  // turn on CTC mode
  TCCR5A |= (1 << WGM51);
  // Set CS51 bit for 8 prescaler
  TCCR5B |= (1 << CS51);   
  // enable timer compare interrupt
  TIMSK5 |= (1 << OCIE5A);
}

unsigned int poll_tachometer(void){
      return tachometer_rpm;
}


ISR(TIMER5_COMPA_vect){//timer5 interrupt 

// This happens at 8khz rate

  tachometer_poll_count++;
  if(tachometer_poll_count >= 8000){

      // this happens at 1Hz rate

      //5 Sample Moving Average To Smooth Out The Data
      tachometer_rpm_array[0] = tachometer_rpm_array[1];
      tachometer_rpm_array[1] = tachometer_rpm_array[2];
      tachometer_rpm_array[2] = tachometer_rpm_array[3];
      tachometer_rpm_array[3] = tachometer_rpm_array[4];
      tachometer_rpm_array[4] = 60*tachometer_count;    
      
      //Last 5 Average RPM Counts Equals....
      tachometer_rpm = (
            tachometer_rpm_array[0] + 
            tachometer_rpm_array[1] + 
            tachometer_rpm_array[2] + 
            tachometer_rpm_array[3] + 
            tachometer_rpm_array[4]
            ) / 5;
      
      // Reset counters
      tachometer_count=0;
      tachometer_poll_count=0; 
  }
}

// tachometer pulse interupt

void pin_itr_tachometer(void){
      tachometer_count++;
}