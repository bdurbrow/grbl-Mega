# What this is:

This is a fork of the version of grbl for the ATMega2560; it adds support for running standalone on common 3D printer hardware.

# WARNING: PRE-BETA SOFTWARE!

This is currently pre-beta. It probably has bugs. It might not work for you. It might crash your CNC. It might turn your dog into a geranium. I really don't know. Use at your own risk.




The original development thread can be found here:

https://github.com/gnea/grbl-Mega/issues/77



## Hardware requirements as distributed:

* A suitable SD card. (Anything that works with other Arduino SD software/hardware - like a 3D printer - should work... I'm using SD drivers originally sourced from the Arduino codebase). Make sure you format it correctly, though, with the tool from the SD Association (https://www.sdcard.org/downloads/formatter_4/index.html). I've had mixed luck using the built-in formatting tools provided by the OS (both OS X and Windows) on SD cards... sometimes it works, sometimes it doesn't. Using the tool from sdcard.org seems to avoid this issue.
* A mega2560.
* A RAMPS 1.4 w/ stepper drivers. Other RAMPS versions might work, but have not been tested.
* A RepRap Discount Smart Controller LCD interface w/ Smart Adapter (the one with the 20x4 character LCD, rotary encoder w/push button, extra push button and SD card slot; **NOT** the ones with a graphic LCD, this firmware doesn't - and won't - support those).
* A 4x4 matrix keypad (there are inexpensive membrane-style ones available on eBay, search for "Arduino keypad 4x4" and pick one - or 10 - that's cheap; and if you get the same one that I got, I've got a PDF file that can be printed out and glued over the keypad to show what the key functions are)
* A suitable MPG handwheel. (The ones I'm using are 100ppr, 60mm handwheels, and can be found on eBay by searching for "CNC MPG 5v 60mm"; at the moment they are under $20usd).
* Switches for Cycle Start and Feed Hold (I'm using some arcade-style push-buttons, again sourced from eBay) and 5K pots for feed rate override and spindle override. These components will require soldering, so make sure you've got a suitable soldering station (temperature controlled iron, "helping hands" to hold stuff, magnification, good solder and flux, etc).
* Suitable cabling to attach everything. (I ordered some long male-to-female 0.100in "DuPont" style hookup wires, and some 1x2, 1x3, 1x8, 2x4, and 2x5 "DuPont" style shrouds from eBay; also I had on hand some suitable ring terminals for hooking up the MPG handwheel).
* And, of course, your CNC machine itself. 😁

Other hardware may be compatible, but has not been tested.

Also, expect to have to check and modify config.h and cpu_map.h for your specific hardware; not all options are turned on in the as-downloaded setup.


## Documentation:

... needs to be written. For right now, there's some information in the UI Support folder, some in this repository's wiki, and also check out the config.h and cpu_map.h files for machine-specific setup.

This image (from Reprap.org) may be helpful in figuring out what I/O pin is what:

https://reprap.org/mediawiki/images/c/ca/Arduinomega1-4connectors.png


## Compiling:
Use the Arduino IDE to compile this; the main file is grbl.ino which is located in the grbl folder.

## December 1, 2019 Bugfix:

* Fixed handling of comments in G-code files.

## October 2019 Update:

* Compiling Grbl is now slightly different - it compiles like any other Arduino sketch. So; remove your old links to the Grbl folder from your Arduino libraries folder; and just open grbl.ino; select your board from the Tools menu, and click the "Upload" button.

* A `platformio.ini` file has been added courtesy of Tim Hawkins. So, if you have PlatformIO installed and want to, you can use that also.

* The makefile was completely out of date; and was not going to be fixed anytime soon. It has been removed. To build this the way that it's been tested, use the Arduino IDE; which is both free and cross-platform.

* Storing system settings from the serial port is now a little bit different than the stock Grbl distribution. After you have configured your machine the way you want; send a $255=1 command via the serial port. This will save your settings to the EEPROM. Otherwise, any settings you change will only be changed in RAM, and will revert to the previous stored values upon the next reset/power cycle. Editing settings & storing them from the System menu works as before.

* The override status display page now shows not only the percent override; but the actual values that will be applied. For example, if you have the spindle set to 1000 RPM, and have the spindle override set to 110%, the LCD will display `Spindle: 110% (1100)`. The display will be updated as the relevant parameters are updated.

* Support for displaying the actual current velocity of the axes on the last page of the status display has been added. This can be disabled in the `config.h` file if you don't want it.

* Support for a spindle tachometer has been added. This is mapped to Arduino pin D11; which is on the RAMPS 1.4 servo header. Configuration options for this are in both the `cpu_map.h` file (where you can set it up to ignore rising or falling edges of the input pulse) and the `config.h` file (where you can completely disable it if you don't have a tachometer sensor, or just disable display of the spindle RPM). This implementation is currently reading a little bit fast; but I wanted to get this update out the door - I intend to continue working on the core tachometer code to get it better. 

* Support for an analog spindle load sensor has been added. The default configuration for this puts the input on Analog channel 4; which on a RAMPS 1.4 card is located on the AUX 1 header. This can be configured in the `config.h` file. By default this is disabled, but can be enabled by uncommenting the `#define UI_SPINDLE_LOAD 4` directive in `config.h`. Note that this pin is the same one used by default for the Laser mode output; and one or the other must be disabled, or different pins must be configured.  

* Laser mode now has a pin that's turned on when active. By default, it's on Arduino pin D58; which on a RAMPS 1.4 card is located on the AUX 1 header. By default this is disabled, but can be enabled by uncommenting the `#define USE_LASER_MODE_ACTIVE_OUTPUT` directive in `cpu_map.h`. 

* The spindle output waveform now has a mode that is compatible with RC Electronic Speed Control modules (such as those commonly used with brushless motors on radio-control aircraft and cars). Enable it by uncommenting the `#define SPINDLE_RC_ESC_MODE` directive on line 394 of `cpu_map.h`.

* Preliminary support for axis cloning and auto-squaring is present; but mostly untested. Output bit inversion is known to be missing; and usage documentation needs to be written.

* Keypads can now have rotation; this can be configured in `config.h` (see line 641).

* Now ignores lines that begin with the "%" character.

* Pressing button 1 on UI Encoder A from the status screen now activates the main menu. With the default configuration this button is the push action on the encoder knob of the RepRap Discount Smart Controller LCD board.


## Targets for future updates (in no particular order):

* Easier per-machine configuration across updates.

* Better accuracy on the spindle tachometer.

* Closed loop spindle RPM control. This probably won't be like having an actual servo motor for a spindle drive; but it will help keep the spindle turning at the speed you asked for.

* MODBUS support on UART 2. The idea is to be able to set the frequency of a VFD for controlling the spindle motor.

* Better support for multi-step operations from C functions.

* Remove the SD card dependency from the Conversational Operations.

* Plasma Cutter Mode with Torch Height Control. Support for both torch up and down input pins as well as analog input is planned. If you are designing hardware and wish to take advantage of the analog input, it will have a 0 to 2.5 volt range; the same as all the other analog inputs that this firmware supports. Probably obvious, but having an opto-isolator somewhere in between the plasma cutter and the Arduino would probably be a good idea.

* Wire EDM Mode. Basically, if it's got good sparking; don't step the steppers. I might also be able to do something for sinker EDMs.

* Lathe mode with threading & SFPM spindle speed control.

* PWM output for the coolant signals.

* Streaming to a SD card file. The idea is to be able to transfer a program to the SD card via a WiFi connection; and I figure most people will use either an ESP8266 or an ESP32 to handle the WiFi part.

***

![GitHub Logo](https://github.com/gnea/gnea-Media/blob/master/Grbl%20Logo/Grbl%20Logo%20250px.png?raw=true)
***
***

Grbl is a no-compromise, high performance, low cost alternative to parallel-port-based motion control for CNC milling. This version of Grbl runs on an Arduino Mega2560 only.

The controller is written in highly optimized C utilizing every clever feature of the AVR-chips to achieve precise timing and asynchronous operation. It is able to maintain up to 30kHz of stable, jitter free control pulses.

It accepts standards-compliant g-code and has been tested with the output of several CAM tools with no problems. Arcs, circles and helical motion are fully supported, as well as, all other primary g-code commands. Macro functions, variables, and most canned cycles are not supported, but we think GUIs can do a much better job at translating them into straight g-code anyhow.

Grbl includes full acceleration management with look ahead. That means the controller will look up to 24 motions into the future and plan its velocities ahead to deliver smooth acceleration and jerk-free cornering.

* [Licensing](https://github.com/gnea/grbl/wiki/Licensing): Grbl is free software, released under the GPLv3 license.

* For more information and help, check out our **[Wiki pages!](https://github.com/gnea/grbl/wiki)** If you find that the information is out-dated, please to help us keep it updated by editing it or notifying our community! Thanks!

* Lead Developer: Sungeun "Sonny" Jeon, Ph.D. (USA) aka @chamnit

* Built on the wonderful Grbl v0.6 (2011) firmware written by Simen Svale Skogsrud (Norway).

***

### Official Supporters of the Grbl CNC Project
![Official Supporters](https://github.com/gnea/gnea-Media/blob/master/Contributors.png?raw=true)


***

##Update Summary for v1.1
- **IMPORTANT:** Your EEPROM will be wiped and restored with new settings. This is due to the addition of two new spindle speed '$' settings.

- **Real-time Overrides** : Alters the machine running state immediately with feed, rapid, spindle speed, spindle stop, and coolant toggle controls. This awesome new feature is common only on industrial machines, often used to optimize speeds and feeds while a job is running. Most hobby CNC's try to mimic this behavior, but usually have large amounts of lag. Grbl executes overrides in realtime and within tens of milliseconds.

- **Jogging Mode** : The new jogging commands are independent of the g-code parser, so that the parser state doesn't get altered and cause a potential crash if not restored properly. Documentation is included on how this works and how it can be used to control your machine via a joystick or rotary dial with a low-latency, satisfying response.

- **Laser Mode** : The new "laser" mode will cause Grbl to move continuously through consecutive G1, G2, and G3 commands with spindle speed changes. When "laser" mode is disabled, Grbl will instead come to a stop to ensure a spindle comes up to speed properly. Spindle speed overrides also work with laser mode so you can tweak the laser power, if you need to during the job. Switch between "laser" mode and "normal" mode via a `$` setting.

	- **Dynamic Laser Power Scaling with Speed** : If your machine has low accelerations, Grbl will automagically scale the laser power based on how fast Grbl is traveling, so you won't have burnt corners when your CNC has to make a turn! Enabled by the `M4` spindle CCW command when laser mode is enabled!

- **Sleep Mode** : Grbl may now be put to "sleep" via a `$SLP` command. This will disable everything, including the stepper drivers. Nice to have when you are leaving your machine unattended and want to power down everything automatically. Only a reset exits the sleep state.

- **Significant Interface Improvements**: Tweaked to increase overall performance, include lots more real-time data, and to simplify maintaining and writing GUIs. Based on direct feedback from multiple GUI developers and bench performance testing. _NOTE: GUIs need to specifically update their code to be compatible with v1.1 and later._

	- **New Status Reports**: To account for the additional override data, status reports have been tweaked to cram more data into it, while still being smaller than before. Documentation is included, outlining how it has been changed. 
	- **Improved Error/Alarm Feedback** : All Grbl error and alarm messages have been changed to providing a code. Each code is associated with a specific problem, so users will know exactly what is wrong without having to guess. Documentation and an easy to parse CSV is included in the repo.
	- **Extended-ASCII realtime commands** : All overrides and future real-time commands are defined in the extended-ASCII character space. Unfortunately not easily type-able on a keyboard, but helps prevent accidental commands from a g-code file having these characters and gives lots of space for future expansion.
	- **Message Prefixes** : Every message type from Grbl has a unique prefix to help GUIs immediately determine what the message is and parse it accordingly without having to know context. The prior interface had several instances of GUIs having to figure out the meaning of a message, which made everything more complicated than it needed to be.

- New OEM specific features, such as safety door parking, single configuration file build option, EEPROM restrictions and restoring controls, and storing product data information.
 
- New safety door parking motion as a compile-option. Grbl will retract, disable the spindle/coolant, and park near Z max. When resumed, it will perform these task in reverse order and continue the program. Highly configurable, even to add more than one parking motion. See config.h for details.

- New '$' Grbl settings for max and min spindle rpm. Allows for tweaking the PWM output to more closely match true spindle rpm. When max rpm is set to zero or less than min rpm, the PWM pin D11 will act like a simple enable on/off output.

- Updated G28 and G30 behavior from NIST to LinuxCNC g-code description. In short, if a intermediate motion is specified, only the axes specified will move to the stored coordinates, not all axes as before.

- Lots of minor bug fixes and refactoring to make the code more efficient and flexible.




```
List of Supported G-Codes in Grbl v1.1:
  - Non-Modal Commands: G4, G10L2, G10L20, G28, G30, G28.1, G30.1, G53, G92, G92.1
  - Motion Modes: G0, G1, G2, G3, G38.2, G38.3, G38.4, G38.5, G80
  - Feed Rate Modes: G93, G94
  - Unit Modes: G20, G21
  - Distance Modes: G90, G91
  - Arc IJK Distance Modes: G91.1
  - Plane Select Modes: G17, G18, G19
  - Tool Length Offset Modes: G43.1, G49
  - Cutter Compensation Modes: G40
  - Coordinate System Modes: G54, G55, G56, G57, G58, G59
  - Control Modes: G61
  - Program Flow: M0, M1, M2, M30*
  - Coolant Control: M7*, M8, M9
  - Spindle Control: M3, M4, M5
  - Valid Non-Command Words: F, I, J, K, L, N, P, R, S, T, X, Y, Z
```

-------------
Grbl is an open-source project and fueled by the free-time of our intrepid administrators and altruistic users. If you'd like to donate, all proceeds will be used to help fund supporting hardware and testing equipment. Thank you!

[![Donate](https://www.paypalobjects.com/en_US/i/btn/btn_donate_LG.gif)](https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=CUGXJHXA36BYW)
