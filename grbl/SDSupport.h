#ifndef SDSUPPORT_H
#define SDSUPPORT_H

  #include <stdint.h>

  #define SD_state_Unmounted (0)
  #define SD_state_Mounted (1)
  #define SD_state_Open (2)
  #define SD_state_Error (255)

  #define SD_DIRECTORY_SIZE (32)

  typedef struct
  {
    char name[8];
  } SD_Directory_Entry;
  extern uint8_t SD_state;
  extern bool grbltempIsOpen;
  extern const char *grbltempDisplayName;    // Display name for when grbltemp is the file being executed. Must be located in program space.
  extern SD_Directory_Entry SD_directory[SD_DIRECTORY_SIZE];
  extern uint8_t SD_directory_count;
  
  #ifdef DISPLAY_SD_LINE_COUNT
    extern int32_t SD_line_count;
  #endif
  
  #ifdef __cplusplus
    
    #include "SD_grbl.h"

    extern File gcode_file;

    extern "C"{
  #endif

    bool SD_mount_card();
    bool SD_open_filename(char *filename);
    bool SD_open_grbltemp(const char *displayName);
    void SD_close();
    void SD_rewind();
    int16_t SD_readGCodeLine(void* buf, uint16_t nbyte);
    void SDParseError();
  
  #ifdef __cplusplus
    } // extern "C"
  #endif

#endif
