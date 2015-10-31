#pragma once

#include <cstddef>

enum class EImageFormat {
    Unknown,
    IntelHex,
    Elf
};

/** Class responsible for reading from .hex and .elf files to unsigned char* buffer */
class ImageManager {
    /** Calls fopen inside and performs error checking */
    static FILE* OpenFile(const char *FileName, const char *mode);

    /** Returns image format */
    static EImageFormat DetermineImageFormat(FILE *Image);

    /** Reads bytes from elf image to buffer. In case of fail prints info to stdout and terminates the program */
    static void ReadElfImage(FILE *Image, unsigned char* buffer, size_t buffer_size);
    
    /** Reads bytes from hex image to buffer. In case of fail prints info to stdout and terminates the program */
    static void ReadHexImage(FILE *Image, unsigned char* buffer, size_t buffer_size);

public:
    /** Reads bytes from image to buffer. In case of fail prints info to stdout and terminates the program */
    static void ReadImage(const char* FileName, unsigned char* buffer, size_t buffer_size);

    /** Writes bytes from buffer to hex image. In case of fail prints info to stdout and terminates the program */
    static void WriteHexImage(const char* FileName, const unsigned char* buffer, size_t buffer_size);
};
