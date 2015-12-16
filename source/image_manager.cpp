#include <iostream>
#include <cstdlib>

#include "image_manager.h"

FILE* ImageManager::OpenFile(const char *FileName, const char *mode) {
    if (FileName == nullptr) {
        std::cout << "emulator: fatal error: no file name\n";
        exit(1);
    }

    FILE *Image = fopen(FileName, mode);
    if (Image == nullptr) {
        std::cout << "emulator: fatal error: coudn't open file named " << FileName << "\n";
        exit(1);
    }

    return Image;
}
    
void ImageManager::ReadHexImage(const char* FileName, uint8_t* buffer, size_t buffer_size) {
    FILE* Image = OpenFile(FileName, "r");

    uint32_t line_number = 1;
    uint32_t address_offset = 0;

    while (true) {
        uint32_t data_size, address, data_type;

        if (fscanf(Image, " :%02x", &data_size) != 1) { std::cout << "emulator: fatal error: hex file is corrupted, line " << line_number << "\n"; exit(1); }
        if (fscanf(Image, "%04x",   &address) != 1)   { std::cout << "emulator: fatal error: hex file is corrupted, line " << line_number << "\n"; exit(1); }
        if (fscanf(Image, "%02x",   &data_type) != 1) { std::cout << "emulator: fatal error: hex file is corrupted, line " << line_number << "\n"; exit(1); }

        uint32_t control_sum = data_size + address + data_type;

        switch (data_type) {
            case 0:
                for (uint32_t i = 0; i < data_size; ++i) {
                    uint32_t absolute_offset = address_offset + address + i;
                    if (absolute_offset >= buffer_size) {
                        std::cout << "emulator: fatal error: image is too big for attiny13a\n";
                        exit(1);
                    }

                    uint8_t *temp = buffer + absolute_offset;
                    if (fscanf(Image, "%02hhx", temp) != 1) { std::cout << "emulator: fatal error: hex file is corrupted, line " << line_number << "\n"; exit(1); }
                    control_sum += *temp;
                }

                break;

            case 1:
                break;

            case 2:
                if (fscanf(Image, "%04x", &address_offset) != 1) { std::cout << "emulator: fatal error: hex file is corrupted, line " << line_number << "\n"; exit(1); }
                address_offset *= 16;
                break;

            case 4:
                if (fscanf(Image, "%04x", &address_offset) != 1) { std::cout << "emulator: fatal error: hex file is corrupted, line " << line_number << "\n"; exit(1); }
                address_offset <<= 16;
                break;

            default:
                std::cout << "emulator: fatal error: hex file is corrupted, line " << line_number << "\n";
                exit(1);
        }

        uint32_t control;
        if (fscanf(Image, "%02x", &control) != 1) { std::cout << "emulator: fatal error: hex file is corrupted, line " << line_number << "\n"; exit(1); }
        control_sum += control;

        if (control_sum % 256 != 0) {
            std::cout << "emulator: fatal error: hex file is corrupted, line " << line_number << ", control sum is " << control_sum % 256 << "\n";
            exit(1);
        }

        if (data_type == 1) {
            fclose(Image);
            return;
        }

        ++line_number;
    }
}

void ImageManager::WriteHexImage(const char* FileName, const uint8_t* buffer, size_t buffer_size) {
    FILE* Image = OpenFile(FileName, "w");

    // As we are going to save only eeprom which consists only of 64 bytes the following code is OK
    uint8_t control_sum = 0x40u;
    fprintf(Image, ":40000000");

    for (size_t i = 0; i < buffer_size; ++i) {
        fprintf(Image, "%02hhx", buffer[i]);
        control_sum += buffer[i];
    }

    fprintf(Image, "%02hhx\n", ~control_sum + 1);
    fprintf(Image, ":00000001FF\n");

    fclose(Image);
}
