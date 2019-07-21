/*

 SD - a slightly more friendly wrapper for sdfatlib

 This library aims to expose a subset of SD card functionality
 in the form of a higher level "wrapper" object.

 License: GNU General Public License V3
          (Because sdfatlib is licensed with this.)

 (C) Copyright 2010 SparkFun Electronics

 */

#ifndef __SD_H__
#define __SD_H__

#include "DriverUtilities.h"

#include "SdFat_grbl.h"
#include "SdFatUtil_grbl.h"

#define FILE_READ O_READ
#define FILE_WRITE (O_READ | O_WRITE | O_CREAT | O_APPEND)

namespace SDLib {

  class File {
    private:
      char _name[13]; // our name
      int8_t _extentionOffset; // number of bytes to add to _name to find the file extension.
      SdFile _file;  // underlying file instance.
  
    protected:
      inline void setWriteError(void) { _file.setWriteError(); }
    
    public:
      File(SdFile &f, const char *n) {  // wraps an underlying SdFile
          _file = f;
          _extentionOffset = 0;
    
          char *destination = _name;
          for(const char *cursor = n; (cursor < (n + 12)); cursor++, destination++)
          {
            if (*cursor == 0) break;
            if ((*destination = *cursor) == '.') _extentionOffset = (destination - _name) + 1;
          };
          *destination = 0;
      }
      File(void) : _extentionOffset(0), _file() { _name[0]=0; }  // 'empty' constructor
      
      ~File(void) { _file.close(); }
      
      inline size_t write(uint8_t val) { return write(&val, 1); }
      inline size_t write(const uint8_t *buf, size_t size)
      {
        _file.clearWriteError();
        return _file.write(buf, size);
      }
      
      inline void writeEOL(void) { _file.writeEOL(); }

      inline void writeGCodeWord_i(char word, uint8_t value)
      {
        char buffer[5];
        buffer[0] = word;
        itoa(value, buffer + 1, 10);
        _file.write(buffer);
      }

      inline void writeGCodeWord_ii(char word, uint8_t value, uint8_t subValue)
      {
        char buffer[9];
        buffer[0] = word;
        itoa(value, buffer + 1, 10);
        char *cursor = buffer + strlen(buffer);
        *cursor++ = '.';
        itoa(subValue, cursor, 10);
        _file.write(buffer);
      }

      inline void writeGCodeWord_l(char word, int32_t value)
      {
        char buffer[20];
        buffer[0] = word;
        ltoa(value, buffer + 1, 10);
        _file.write(buffer);
      }

      inline void writeGCodeWord_f(char word, float value)
      {
        char buffer[20];
        buffer[0] = word;
        uint8_t length = sPrintFloat(buffer + 1, 18, value, 4) - buffer;
        _file.write(buffer, length);
      }
      
      inline int16_t read() { return _file.read(); }
      inline int16_t read(void *buf, uint16_t nbyte) { return _file.read(buf, nbyte); }
      inline int16_t readGCodeLine(void* buf, uint16_t nbyte) { return _file.readGCodeLine(buf, nbyte); }
      inline int available()
      {
        if (!_file.isOpen()) return 0;
        uint32_t n = size() - position();
        return n > 0X7FFF ? 0X7FFF : n;
      }
      
      inline int peek()
      {
        if (!_file.isOpen()) return 0;
        int c = _file.read();
        if (c != -1) _file.seekCur(-1);
        return c;
      }
      
      inline void flush() { _file.sync(); }
      
      inline bool seek(uint32_t pos) { return _file.seekSet(pos); }
      
      
      inline uint32_t position() { return _file.curPosition(); }
      inline uint32_t size() { return _file.fileSize(); }
      inline void close() { _file.close(); }
      inline operator bool() { return _file.isOpen(); }
      
      inline char *name(void) { return _name; }
      inline char *extension(void) { return _name + _extentionOffset; };
      inline boolean isDirectory(void) { return _file.isDir(); }

      File openNextFile(uint8_t mode = O_RDONLY);
      void rewindDirectory(void);
      
      inline bool getWriteError() { return _file.getWriteError(); }
      inline void clearWriteError() { _file.clearWriteError(); }
  };

  class SDClass {

    private:
      // These are required for initialisation and use of sdfatlib
      Sd2Card card;
      SdVolume volume;
      SdFile root;
  
      // my quick&dirty iterator, should be replaced
      SdFile getParentDir(const char *filepath, int *indx);
    public:
      // This needs to be called to set up the connection to the SD card
      // before other methods are used.
      boolean begin(void);
      boolean begin(uint32_t clock);
  
      //call this when a card is removed. It will allow you to insert and initialise a new card.
      void end();

      // Open the specified file/directory with the supplied mode (e.g. read or
      // write, etc). Returns a File object for interacting with the file.
      // Note that currently only one file can be open at a time.
      File open(const char *filename, uint8_t mode = FILE_READ);

      // Methods to determine if the requested file path exists.
      boolean exists(const char *filepath);

      // Create the requested directory heirarchy--if intermediate directories
      // do not exist they will be created.
      boolean mkdir(const char *filepath);
  
      // Delete the file.
      boolean remove(const char *filepath);
  
      boolean rmdir(const char *filepath);

    private:

      // This is used to determine the mode used to open a file
      // it's here because it's the easiest place to pass the 
      // information through the directory walking function. But
      // it's probably not the best place for it.
      // It shouldn't be set directly--it is set via the parameters to `open`.
      int fileOpenMode;
  
      friend class File;
      friend boolean callback_openPath(SdFile&, const char *, boolean, void *); 
  };

  extern SDClass SD;

};

// We enclose File and SD classes in namespace SDLib to avoid conflicts
// with others legacy libraries that redefines File class.

// This ensure compatibility with sketches that uses only SD library
using namespace SDLib;

// This allows sketches to use SDLib::File with other libraries (in the
// sketch you must use SDFile instead of File to disambiguate)
typedef SDLib::File    SDFile;
typedef SDLib::SDClass SDFileSystemClass;
#define SDFileSystem   SDLib::SD

#endif
