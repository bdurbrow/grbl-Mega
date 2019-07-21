/*
  grbl_expansion_interface.c
  Part of Grbl

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

#include "grbl.h"

void gxi_set_callback(struct parser_block_t *block, gxi_callback_t callbackFunction, gxi_callback_context_t callbackContext)
{
  block->callbackFunction = callbackFunction;
  block->callbackContext = callbackContext;
};

// Called to validate otherwise unsupported M codes.
// If the specified M code will be handled, add a case statement for it is added to the gxi_M_codes() macro
// to cause this to return STATUS_OK. The M code value will then be available to any functions listed in the
// gxi_will_execute_block macro in gxi_block->values.gxi_mcode
uint8_t gxi_validate_M_code(uint8_t mcode)
{
  switch(mcode)
  {
    #ifdef gxi_M_codes
      gxi_M_codes();
        return STATUS_OK;
    #endif
    default: return STATUS_GCODE_UNSUPPORTED_COMMAND;
  }
}
