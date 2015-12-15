#pragma once

#include <cstddef>
#include <stdint.h>

/** Class responsible for reading from .hex and .elf files to unsigned char* buffer */
class ImageManager {
    /** Calls fopen inside and performs error checking */
    static FILE* OpenFile(const char *FileName, const char *mode);
    
public:
    /** Reads bytes from image to buffer. In case of fail prints info to stdout and terminates the program */
    static void ReadHexImage(const char* FileName, uint8_t* buffer, size_t buffer_size);

    /** Writes bytes from buffer to hex image. In case of fail prints info to stdout and terminates the program */
    static void WriteHexImage(const char* FileName, const uint8_t* buffer, size_t buffer_size);
};
