#pragma once

#include <vector>
#include <cstdio>
#include <ctime>

/** All consts lie here */
struct HardwareInfo {
    // Ammount of memory in bytes
    size_t FlashMemory, EEPROM, SRAM;

    HardwareInfo() : FlashMemory(1024),
                     EEPROM(64),
                     SRAM(64) { }
};

/** Log mode enum */
enum ELogMode {
    Silent     = 0,
    Errors     = (1 << 0),
    Warnings   = (1 << 1),
    Interrupts = (1 << 2),
    InOut      = (1 << 3),
    All        = (1 << 4)
};

/** Emulation params */
struct RunParams {
    FILE *logfile;
    FILE *infile, *outfile;

    time_t lifetime;
    ELogMode LogMode;

    RunParams() : logfile(stdout),
                  infile(nullptr),
                  outfile(nullptr),
                  lifetime(1000),
                  LogMode(ELogMode::Silent) { }
};

/** Emulator of ATtiny13A */
class Emulator {
    std::vector<unsigned short> FlashMemory;
    std::vector<unsigned char> EEPROM, SRAM;
    unsigned short PC;

    HardwareInfo Info;
    RunParams Params;

    /** Checks for interrupt and launch ISR if needed. Returns true if interrupt happened */
    bool CheckForInterrupt();

    /** Log */
    void log(ELogMode LogMode, const char* message);

public:
    /** Creates an emulator and uploads flash and EEPROM images in it */
    Emulator(const std::vector<unsigned short> &InFlashMemory, const std::vector<unsigned char> &InEEPROM, const RunParams& InParams);

    Emulator(const Emulator&) = delete;
    Emulator& operator=(const Emulator&) = delete;

    /** Runs an emulator (in the current thread) */
    void Run();

    /** Processes the next instruction */
    void ProcessInstruction();

    /** Retrieve info about hardware */
    inline const HardwareInfo& GetHardwareInfo() const { return Info; }

    /** Get image of flash memory */
    inline const std::vector<unsigned short>& GetFlashMemory() const { return FlashMemory; }

    /** Get image of EEPROM */
    inline const std::vector<unsigned char>& GetEEPROM() const { return EEPROM; }

    /** Get image of SRAM */
    inline const std::vector<unsigned char>& GetSRAM() const { return SRAM; }

    /** Get PC */
    inline unsigned short GetPC() const { return PC; }
};
