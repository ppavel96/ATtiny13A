#pragma once

#include <string>
#include <vector>
#include <cstdio>
#include <ctime>

/** Log mode */
struct LogFlags {
    uint32_t Interrupt : 1;
    uint32_t InOut : 1;
    uint32_t Detailed : 1;

    LogFlags() : Interrupt(0),
                 InOut(0),
                 Detailed(0) {}
};

/** Emulation params */
struct RunParams {
    FILE *logfile, *infile, *outfile;
    LogFlags LogMode;
    time_t lifetime;

    RunParams() : logfile(stdout),
                  infile(nullptr),
                  outfile(nullptr),
                  lifetime(10) {}
};

/** Emulator sleep modes */
enum class EmulatorState {
    Working,
    Idle,
    ADCNoiseReduction,
    PowerDown
};

/** Instruction of AVR microcontroller */
class Instruction {
    std::string Name, Opcode;
    uint16_t Size;

    void (*Func)(class Emulator &ATtiny13A);

public:
    /** Creates instruction with a given name, opcode and function to execute */
    Instruction(std::string InName, std::string InOpcode, uint16_t InSize, void (*InFunc)(class Emulator &ATtiny13A));

    /** Checks if opcode matches and executes Func */
    bool TryExecute(class Emulator &ATtiny13A);
};

/** Emulator of ATtiny13A */
class Emulator {
    std::vector<uint16_t> FlashMemory;
    std::vector<uint8_t> EEPROM, SRAM;
    std::vector<Instruction> InstructionSet;
    uint16_t PC;

    RunParams Params;
    EmulatorState State;

    /** Whether next instruction should be skipped */
    bool Skip;

    /** When the AVR exits from an interrupt, it will always return to the main program and execute one more instruction before any pending interrupt is served */
    bool CanInterrupt;

    /** Checks for interrupt and launch ISR if needed */
    void CheckForInterrupt();

public:
    /** Creates an emulator and uploads flash and EEPROM images in it */
    Emulator(const std::vector<uint16_t> &InFlashMemory, const std::vector<uint8_t> &InEEPROM, const RunParams& InParams);
    Emulator(const Emulator&) = delete;
    Emulator& operator=(const Emulator&) = delete;

    /** Runs an emulator (in the current thread) */
    void Run();

    /** Processes the next instruction */
    void ProcessInstruction();

    /** Get image of flash memory */
    inline const std::vector<uint16_t>& GetFlashMemory() const { return FlashMemory; }

    /** Get image of EEPROM */
    inline const std::vector<uint8_t>& GetEEPROM() const { return EEPROM; }

    /** Get image of SRAM */
    inline const std::vector<uint8_t>& GetSRAM() const { return SRAM; }

    /** Get PC */
    inline uint16_t GetPC() const { return PC; }

    friend Instruction;
};
