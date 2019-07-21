#include "LiquidCrystal_grbl.h"

#include <stdio.h>
#include <string.h>
#include <inttypes.h>

#include "grbl.h"
#include "DriverUtilities.h"

// commands
#define LCD_CLEARDISPLAY 0x01
#define LCD_RETURNHOME 0x02
#define LCD_ENTRYMODESET 0x04
#define LCD_DISPLAYCONTROL 0x08
#define LCD_CURSORSHIFT 0x10
#define LCD_FUNCTIONSET 0x20
#define LCD_SETCGRAMADDR 0x40
#define LCD_SETDDRAMADDR 0x80

// flags for display entry mode
#define LCD_ENTRYRIGHT 0x00
#define LCD_ENTRYLEFT 0x02
#define LCD_ENTRYSHIFTINCREMENT 0x01
#define LCD_ENTRYSHIFTDECREMENT 0x00

// flags for display on/off control
#define LCD_DISPLAYON 0x04
#define LCD_DISPLAYOFF 0x00
#define LCD_CURSORON 0x02
#define LCD_CURSOROFF 0x00
#define LCD_BLINKON 0x01
#define LCD_BLINKOFF 0x00

// flags for display/cursor shift
#define LCD_DISPLAYMOVE 0x08
#define LCD_CURSORMOVE 0x00
#define LCD_MOVERIGHT 0x04
#define LCD_MOVELEFT 0x00

// flags for function set
#define LCD_8BITMODE 0x10
#define LCD_4BITMODE 0x00
#define LCD_2LINE 0x08
#define LCD_1LINE 0x00
#define LCD_5x10DOTS 0x04
#define LCD_5x8DOTS 0x00

// When the display powers up, it is configured as follows:
//
// 1. Display clear
// 2. Function set: 
//    DL = 1; 8-bit interface data 
//    N = 0; 1-line display 
//    F = 0; 5x8 dot character font 
// 3. Display on/off control: 
//    D = 0; Display off 
//    C = 0; Cursor off 
//    B = 0; Blinking off 
// 4. Entry mode set: 
//    I/D = 1; Increment by 1 
//    S = 0; No shift 
//
// Note, however, that resetting the Arduino doesn't reset the LCD, so we
// can't assume that its in that state when a sketch starts (and the
// LiquidCrystal constructor is called).


/************ low level data pushing commands **********/

static inline void pulseEnable(void)
{
  //fastDigitalWrite(LCD_PIN_EN, LOW);
  //delayMicros(1);    
  fastDigitalWrite(LCD_PIN_EN, HIGH);
  delayMicros(1);    // enable pulse must be >450ns
  fastDigitalWrite(LCD_PIN_EN, LOW);
}

static inline void write4bits(uint8_t value)
{
  fastDigitalWrite(LCD_PIN_D4, value & 0x01); value = value >> 1;
  fastDigitalWrite(LCD_PIN_D5, value & 0x01); value = value >> 1;
  fastDigitalWrite(LCD_PIN_D6, value & 0x01); value = value >> 1;
  fastDigitalWrite(LCD_PIN_D7, value & 0x01); value = value >> 1;

  pulseEnable();
}

// write either command or data
static inline void send(uint8_t value, uint8_t mode)
{
  fastDigitalWrite(LCD_PIN_RS, mode);

  write4bits(value>>4);
  write4bits(value);
  
  st_prep_buffer();
  delayMicros(LCD_SEND_DELAY);
}

/************ Initialization **********/

void LiquidCrystal::begin()
{
  setPinMode(LCD_PIN_RS, OUTPUT);
  setPinMode(LCD_PIN_EN, OUTPUT);
  setPinMode(LCD_PIN_D4, OUTPUT);
  setPinMode(LCD_PIN_D5, OUTPUT);
  setPinMode(LCD_PIN_D6, OUTPUT);
  setPinMode(LCD_PIN_D7, OUTPUT);
  
  // SEE PAGE 45/46 FOR INITIALIZATION SPECIFICATION!
  // according to datasheet, we need at least 40ms after power rises above 2.7V
  // before sending commands. Arduino can turn on way before 4.5V so we'll wait 50
  delayMicros(50000); 
  // Now we pull both RS and R/W low to begin commands
  fastDigitalWrite(LCD_PIN_RS, LOW);
  fastDigitalWrite(LCD_PIN_EN, LOW);
  
  //put the LCD into 4 bit mode
    // this is according to the hitachi HD44780 datasheet
    // figure 24, pg 46

    // we start in 8bit mode, try to set 4 bit mode
    write4bits(0x03);
    delayMicros(4500); // wait min 4.1ms

    // second try
    write4bits(0x03);
    delayMicros(4500); // wait min 4.1ms
    
    // third go!
    write4bits(0x03); 
    delayMicros(150);

    // finally, set to 4-bit interface
    write4bits(0x02); 

  // finally, set # lines, font size, etc.
  command(LCD_FUNCTIONSET | LCD_2LINE);  

  // turn the display on with no cursor or blinking default
  _displaycontrol = LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF;  
  display();

  command(LCD_CLEARDISPLAY);  // clear display, set cursor position to zero
  delayMicros(2000);  // this command takes a long time!

  // Initialize to default text direction
  command(LCD_ENTRYMODESET | LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT);

}

/********** high level commands, for the user! */

void LiquidCrystal::setCursor(uint8_t col, uint8_t row)
{
  uint8_t offset;
  switch(row)
  {
    case 0: offset = 0x00;  break;
    case 1: offset = 0x40;  break;
    case 2: offset = 0x14;  break;
    case 3: offset = 0x54;  break;
    default:
      offset = 0x00;
  }
  
  command(LCD_SETDDRAMADDR | (col + offset));
}

// Turn the display on/off (quickly)
void LiquidCrystal::noDisplay()
{
  _displaycontrol &= ~LCD_DISPLAYON;
  command(LCD_DISPLAYCONTROL | _displaycontrol);
}
void LiquidCrystal::display()
{
  _displaycontrol |= LCD_DISPLAYON;
  command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// Turns the underline cursor on/off
void LiquidCrystal::noCursor()
{
  _displaycontrol &= ~LCD_CURSORON;
  command(LCD_DISPLAYCONTROL | _displaycontrol);
}
void LiquidCrystal::cursor()
{
  _displaycontrol |= LCD_CURSORON;
  command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// Turn on and off the blinking cursor
void LiquidCrystal::noBlink()
{
  _displaycontrol &= ~LCD_BLINKON;
  command(LCD_DISPLAYCONTROL | _displaycontrol);
}
void LiquidCrystal::blink()
{
  _displaycontrol |= LCD_BLINKON;
  command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// Allows us to fill the first 8 CGRAM locations
// with custom characters
void LiquidCrystal::createChar(uint8_t location, uint8_t charmap[])
{
  command(LCD_SETCGRAMADDR | (location << 3));
  for (int i=0; i<8; i++) send(charmap[i], HIGH);
}

/*********** mid level commands, for sending data/cmds */

void LiquidCrystal::command(uint8_t value)
{
  send(value, LOW);
}

void LiquidCrystal::write(uint8_t value)
{
  send(value, HIGH);
}

size_t LiquidCrystal::write(const char *buffer)
{
  size_t count = 0;
  char c;
  while((c=*buffer++))
  {
    send(c, HIGH);
    count++;
  };
  return count;
}

void LiquidCrystal::write(const char *buffer, uint8_t length)
{
  for(; length; length--)
    send(*buffer++, HIGH);
}

size_t LiquidCrystal::write_length(const char *buffer, uint8_t length)
{
  size_t count = 0;
  char c;
  while((count < length) && (c=*buffer++))
  {
    send(c, HIGH);
    count++;
  };
  return count;
}

void LiquidCrystal::writeMultiple(uint8_t value, uint8_t count)
{
  for(; count; count--)
    send(value, HIGH);
}

void LiquidCrystal::write_p(const char *buffer)
{
  PGM_P p = reinterpret_cast<PGM_P>(buffer);
  char c;
  while((c=pgm_read_byte(p++)))
    send(c, HIGH);
}

void LiquidCrystal::write_p(const char *buffer, uint8_t length)
{
  PGM_P p = reinterpret_cast<PGM_P>(buffer);
  for(; length; length--)
    send(pgm_read_byte(p++), HIGH);
}

