#ifndef LiquidCrystal_h
#define LiquidCrystal_h

#include <inttypes.h>
#include <stddef.h>

class LiquidCrystal {
public:
    
  void begin();

  void noDisplay();
  void display();
  void noBlink();
  void blink();
  void noCursor();
  void cursor();

  void createChar(uint8_t, uint8_t[]);
  void setCursor(uint8_t, uint8_t); 
  void write(uint8_t);
  void write(const char *buffer, uint8_t length); // Writes the entire buffer of length characters, including any null values found. Buffer must be in SRAM.
  size_t write(const char *buffer); // Writes a null-terminated string from SRAM.
  size_t write_length(const char *buffer, uint8_t length); // Writes a null-terminated string from SRAM, up to the specified number of characters.
  void writeMultiple(uint8_t value, uint8_t count);   // Writes value count times.
  void write_p(const char *buffer);                   // Writes a null-terminated string from program memory.
  void write_p(const char *buffer, uint8_t length);   // Writes a buffer of specified length from program memory.
  void command(uint8_t);
  
private:
  uint8_t _displaycontrol;
};

#endif
