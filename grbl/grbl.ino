/***********************************************************************
This sketch compiles and uploads Grbl to your mega2560-based Arduino! 

To use:

- Select your Arduino Board and Serial Port in the Tools drop-down menu.
  NOTE: Grbl-mega only officially supports ATMega2560-based Arduinos.
  Using other boards will likely not work!

- Then just click 'Upload'. That's it!

For advanced users:
  If you'd like to see what else Grbl can do, there are some additional
  options for customization and features you can enable or disable. 
  Navigate your file system to where the Grbl source code files are stored,  
  and open the 'config.h' file in your favorite text editor. Inside are 
  dozens of feature descriptions and #defines. Simply comment or uncomment
  the #defines or alter their assigned values, save your changes, and then
  click 'Upload' here. 

Copyright (c) 2015 Sungeun K. Jeon
Released under the MIT-license. See license.txt for details.
***********************************************************************/

#include "grbl.h"

// Do not alter this file!
