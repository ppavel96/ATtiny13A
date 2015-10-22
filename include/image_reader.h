#pragma once

#include <cstddef>

/** Class responsible for reading from .hex and .elf files to unsigned char* buffer */
class ImageReader {
public:
	/** Returns true on success */
	static bool ReadImage(const char* FileName, char* buffer, size_t buffer_size);
};
