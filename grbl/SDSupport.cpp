#include "grbl.h"

#include "SDSupport.h"
#include "UISupport.h"

uint8_t SD_state = SD_state_Unmounted;
bool grbltempIsOpen = false;
const char *grbltempDisplayName;

File gcode_file;

SD_Directory_Entry SD_directory[SD_DIRECTORY_SIZE];
uint8_t SD_directory_count = 0;

#ifdef DISPLAY_SD_LINE_COUNT
  int32_t SD_line_count = 0;
#endif

extern "C"
{
  bool SD_mount_card()
  {
    SD_directory_count = 0;

    if(SD.begin())
    {
      File root = SD.open("/");
      if(!root) { goto failure; }
      
      while(SD_directory_count < SD_DIRECTORY_SIZE)
      {
        File entry = root.openNextFile();
        if (!entry) break;
        
        char *extension = entry.extension();
        if ( (((extension[0] == 'N') && (extension[1] == 'C')) || 
              ((extension[0] == 'n') && (extension[1] == 'c'))) &&
              (extension[2] == 0) )
        {
            char *source = entry.name();
            char *destination = SD_directory[SD_directory_count].name;
            for ( uint8_t count = 0; count < 8; count++ )
            {
              if(*source == '.')
              {
                *destination = 0;
                break;
              }
              *destination++ = *source++;
            }
        
            SD_directory_count++;
        }
      }
      
      SD_state = SD_state_Mounted;
      grbltempIsOpen = false;
      return true;
    }
    
    failure:
      SD_state = SD_state_Error;
      grbltempIsOpen = false;

      return false;
  }

  bool SD_open_filename(char *filename)
  {
    #ifdef DISPLAY_SD_LINE_COUNT
      SD_line_count = 0;
    #endif
    
    if(gcode_file) gcode_file.close();
    gcode_file = SD.open(filename);
    if(gcode_file)
    {
      SD_state = SD_state_Open;
      grbltempIsOpen = false;
      return true;
    }
    else
    {
      SD_state = SD_state_Error;
      grbltempIsOpen = false;
      return false;
    }
  }
  
  bool SD_open_grbltemp(const char *displayName)
  {
    #ifdef DISPLAY_SD_LINE_COUNT
      SD_line_count = 0;
    #endif
    
    if(SD_state == SD_state_Unmounted) SD_mount_card();
  
    if(gcode_file) gcode_file.close();
    SD.remove("grbltemp.tmp");
    gcode_file = SD.open("grbltemp.tmp", FILE_WRITE);
    if(gcode_file)
    {
      SD_state = SD_state_Open;
      grbltempIsOpen = true;
      grbltempDisplayName = displayName;
      return true;
    }
    else
    {
      SD_state = SD_state_Error;
      grbltempIsOpen = false;
      return false;
    }
  }

  void SD_close()
  {
    if(gcode_file)
    {
      gcode_file.close();
      SD_state = SD_state_Mounted;
      grbltempIsOpen = false;
    }
    else
    {
      SD_state = SD_state_Error;
      grbltempIsOpen = false;
    }
    UIStatusNeedsUpdate();
  }
  
  void SD_rewind()
  {
    gcode_file.flush();
    if(!gcode_file.seek(0))
    {
      SD_state = SD_state_Error;
      grbltempIsOpen = false;
    }
  }

  int16_t SD_readGCodeLine(void* buf, uint16_t nbyte)
  {
    int16_t result = gcode_file.readGCodeLine(buf, nbyte);
    
    #ifdef DISPLAY_SD_LINE_COUNT
      SD_line_count++;
    #endif
    
    if(result) return result;
    
    protocol_buffer_synchronize();
    SD_state = SD_state_Mounted;
    grbltempIsOpen = false;
    gcode_file.close();
    UIStatusNeedsUpdate();
    return 0;
  }

  void SDParseError()
  {
    SD_state = SD_state_Error;
    grbltempIsOpen = false;
    gcode_file.close();
  }
}