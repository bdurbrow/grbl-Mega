/*
  grbl_expansion_interface.h
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

/*
  This header defines macros that are sprinkled thru the codebase to provide hooks for
  expanding grbl's capabilities. By default, the macros are empty - they expand to nothing.
  
  However, if you want to intercept certain events, you can add your own calls to the macro
  definitions, and they will be inserted into the relevant call points at compile time.
  Prototypes for these functions would be added at the end of this header as well.
  
  Defined in the calling context is a variable, uint8_t gxi_result, that is used to store return
  status during an error sequence.
  
  
  
  For example, to add a function to examine the g-code line about to be executed, you could
  do something like this:
  
  #define gxi_will_parse_line(gxi_line)
            if((gxi_result = my_expansion_function(gxi_line)))  FAIL(gxi_result);
            
  
  .
  .
  .
  
  
  uint8_t my_expansion_function(char *line);
*/


/*
  Adding a custom M code:
  
  By adding to the macro gxi_M_codes() defined below and implementing a function that hooks into
  the gxi_will_execute_block() macro, you can add a custom M code to grbl.
  
  An example might look like this:
  
      #define gxi_will_execute_block(gxi_block, gxi_state, gxi_flags)
                if((gxi_result = my_mcode_function(gxi_block, gxi_state, gxi_flags))) FAIL(gxi_result);

  
      .
      .
      .
  
  
      #define gxi_M_codes()  case 100: case 110:  
  
      .
      .
      .
  
      uint8_t my_mcode_function(parser_block_t *block, parser_state_t *state, uint8_t flags);

  And in a separate .c file:
  
  #include "grbl.h"
  uint8_t my_mcode_function(parser_block_t *block, parser_state_t *state, uint8_t flags)
  {
    if(block->values.gxi_mcode == 100)
    {
      protocol_buffer_synchronize();  // Omit this call to build an asynchronous M code
      
      // Insert code here to do whatever needs to be done to handle this M code.
      // Return a non-zero status code (as defined in report.h) to signal an error.
    };
    
    return STATUS_OK;
  }

*/

#ifndef grbl_expansion_interface_h
#define grbl_expansion_interface_h

  // Add function calls here to be called when grbl is starting up. Note that these may be called
  // more than once in a given power cycle in the case of a soft-reset condition occurring.
  #define gxi_init()                                                                                \



  // Add function calls here to be called when grbl is idle (such as when the protocol buffer is full,
  // or when there's no g-code commands to execute). Note that it is important for these to be 
  // high-performance, as they will impact grbl's responsiveness to incoming commands.
  #define gxi_loop()                                                                                \



  // Add function calls here to be called when grbl will parse a line of g-code.
  // gxi_line is a null-terminated string containing the line to be parsed.
  #define gxi_will_parse_line(gxi_line)                                                             \



  // Add function calls here to be called when grbl will execute a block of g-code.
  // gxi_block is a pointer to a parser_block_t struct
  // gxi_state is a pointer to a parser_state_t struct
  // gxi_flags is a uint8_t bitfield, with values defined in gcode.h
  // Both parser_block_t and parser_state_t are defined in gcode.h
  #define gxi_will_execute_block(gxi_block, gxi_state, gxi_flags)                                   \



  // To add M codes to the g-code line interpreter, uncomment this #define and add case statements
  // for them. For example:  #define gxi_M_codes()  case 100: case 110:
  
  //#define gxi_M_codes()


  // Interrupt-time callback handling.

  typedef union
  {
    void *p;
    uint16_t i;
  } gxi_callback_context_t;

  typedef void (*gxi_callback_t)(gxi_callback_context_t context);

  // Call this from a function called by the gxi_will_execute_block macro set a callback function to be
  // called by the stepper ISR when the g-code block is completed.
  // IMPORTANT: callback functions are called at interrupt time, by the stepper ISR, and MUST execute
  // VERY QUICKLY! Note also that interrupts are enabled at the time that a callback function is executed.
  struct parser_block_t;
  void gxi_set_callback(struct parser_block_t *block, gxi_callback_t function, gxi_callback_context_t callbackContext);




  // Internal Use Functions:
  
  // Called to by the g-code line interpreter to validate otherwise unsupported M codes.
  uint8_t gxi_validate_M_code(uint8_t mcode);

#endif