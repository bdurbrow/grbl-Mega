/*
  eeprom.h - EEPROM methods
  Part of Grbl

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

#ifndef eeprom_h
#define eeprom_h

uint8_t eeprom_get_char(uint16_t addr);
void eeprom_put_char(uint16_t addr, uint8_t new_value);
void memcpy_to_eeprom_with_checksum(uint16_t destination, void *source, uint16_t size);
int16_t memcpy_from_eeprom_with_checksum(void *destination, uint16_t source, uint16_t size);

#endif
