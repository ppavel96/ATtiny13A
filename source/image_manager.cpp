#include <iostream>
#include <cstdlib>

#include "image_manager.h"

FILE* ImageManager::OpenFile(const char *FileName, const char *mode) {
    if (FileName == nullptr) {
        std::cout << "emulator: fatal error: no file name\n";
        exit(0);
    }

    FILE *Image = fopen(FileName, mode);
    if (Image == nullptr) {
        std::cout << "emulator: fatal error: coudn't open file named " << FileName << "\n";
        exit(0);
    }

    return Image;
}

EImageFormat ImageManager::DetermineImageFormat(FILE *Image) {
    return EImageFormat::IntelHex;
}

void ImageManager::ReadImage(const char* FileName, unsigned char* buffer, size_t buffer_size) {
    FILE* Image = OpenFile(FileName, "r");
    EImageFormat Format = DetermineImageFormat(Image);

    switch (Format) {
        case EImageFormat::IntelHex:
            ReadHexImage(Image, buffer, buffer_size);
            break;

        case EImageFormat::Elf:
            ReadElfImage(Image, buffer, buffer_size);
            break;

        case EImageFormat::Unknown:
            std::cout << "emulator: fatal error: file " << FileName << " has incorrect format or is corrupted\n";
            exit(0);
    }

    fclose(Image);
}

void ImageManager::ReadElfImage(FILE *Image, unsigned char* buffer, size_t buffer_size) {
    // Not implemented yet
}
    
void ImageManager::ReadHexImage(FILE *Image, unsigned char* buffer, size_t buffer_size) {
    unsigned int line_number = 1;
    unsigned int address_offset = 0;

    while (true) {
        unsigned int data_size, address, data_type;

        if (fscanf(Image, " :%02x", &data_size) != 1) { std::cout << "emulator: fatal error: hex file is corrupted, line " << line_number << "\n"; exit(0); }
        if (fscanf(Image, "%04x",   &address) != 1)   { std::cout << "emulator: fatal error: hex file is corrupted, line " << line_number << "\n"; exit(0); }
        if (fscanf(Image, "%02x",   &data_type) != 1) { std::cout << "emulator: fatal error: hex file is corrupted, line " << line_number << "\n"; exit(0); }

        unsigned int control_sum = data_size + address + data_type;

        switch (data_type) {
            case 0:
                for (unsigned int i = 0; i < data_size; ++i) {
                    unsigned int absolute_offset = address_offset + address + i;
                    if (absolute_offset >= buffer_size) {
                        std::cout << "emulator: fatal error: image is too big for attiny13a\n";
                        exit(0);
                    }

                    unsigned char *temp = buffer + absolute_offset;
                    if (fscanf(Image, "%02hhx", temp) != 1) { std::cout << "emulator: fatal error: hex file is corrupted, line " << line_number << "\n"; exit(0); }
                    control_sum += *temp;
                }

                break;

            case 1:
                break;

            case 2:
                if (fscanf(Image, "%04x", &address_offset) != 1) { std::cout << "emulator: fatal error: hex file is corrupted, line " << line_number << "\n"; exit(0); }
                address_offset *= 16;
                break;

            case 4:
                if (fscanf(Image, "%04x", &address_offset) != 1) { std::cout << "emulator: fatal error: hex file is corrupted, line " << line_number << "\n"; exit(0); }
                address_offset <<= 16;
                break;

            default:
                std::cout << "emulator: fatal error: hex file is corrupted, line " << line_number << "\n";
                exit(0);
        }

        unsigned int control;
        if (fscanf(Image, "%02x", &control) != 1) { std::cout << "emulator: fatal error: hex file is corrupted, line " << line_number << "\n"; exit(0); }
        control_sum += control;

        if (control_sum % 256 != 0) {
            std::cout << "emulator: fatal error: hex file is corrupted, line " << line_number << ", control sum is " << control_sum % 256 << "\n";
            exit(0);
        }

        if (data_type == 1)
            return;

        ++line_number;
    }
}

void ImageManager::WriteHexImage(const char* FileName, const unsigned char* buffer, size_t buffer_size) {
    // Not implemented yet
}
