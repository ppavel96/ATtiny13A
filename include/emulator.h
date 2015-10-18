#include <cstddef>
#include <vector>

/** All consts lie here */
struct HardwareInfo {
    // Ammount of memory in bytes
    size_t FlashMemory, EEPROM, SRAM;

    HardwareInfo() {
        FlashMemory = 1024;
        EEPROM = 64;
        SRAM = 64;
    }
};

/** Emulator of ATtiny13A */
class Emulator {
    HardwareInfo Info;
    std::vector<char> FlashMemory, EEPROM, SRAM;

public:
    Emulator();

    /** Perform a tick */

    /** Retrieve info about hardware */
    inline const HardwareInfo& GetHardwareInfo() const { return Info; }

    /** Get image of flash memory */
    inline const std::vector<char>& GetFlashMemory() const { return FlashMemory; }

    /** Get image of EEPROM */
    inline const std::vector<char>& GetEEPROM() const { return EEPROM; }

    /** Get image of SRAM */
    inline const std::vector<char>& GetSRAM() const { return SRAM; }
};
