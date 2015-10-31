#include <cstdlib>
#include "emulator.h"

Emulator::Emulator(const std::vector<unsigned short> &InFlashMemory, const std::vector<unsigned char> &InEEPROM, const RunParams& InParams) :
    FlashMemory(InFlashMemory), EEPROM(InEEPROM), Params(InParams) {
    SRAM = std::vector<unsigned char>(Info.SRAM);
    PC = 0;
}

bool Emulator::CheckForInterrupt() {
    return false;
}

void Emulator::log(ELogMode LogMode, const char* message) {
    if (Params.LogMode & LogMode)
        fprintf(Params.logfile, "%s", message);
}

void Emulator::Run() {
    std::clock_t start_time = std::clock();

    while (true) {
        // do some timers, check for input, print logs....
        // ....

        if (!CheckForInterrupt())
            ProcessInstruction();

        if ((std::clock() - start_time) / (double)(CLOCKS_PER_SEC / 1000) > Params.lifetime)
            return;
    }
}

void Emulator::ProcessInstruction() {
    unsigned short instruction = FlashMemory[PC];
    // switch (instruction) ...
    // ...
}
