#include <cstdlib>
#include "emulator.h"

#define BIT_GET(a,b)     (((a) >> b) & 1u)
#define BIT_GET_INV(a,b) ((((a) >> b) & 1u) ^ 1u)

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
    /** PC overflow */
    if (PC >= FlashMemory.size()) {
        char message[500];
        snprintf(message, sizeof(message), "PC overflow: PC=0x%04hx\n", PC);
        log(ELogMode::Errors, message);

        Params.lifetime = -1;
        return;
    }

    /** Get instruction */
    unsigned short instruction = FlashMemory[PC];
    
    /** ADC: 0001 11rd dddd rrrr */
    if ((instruction & 0xFC00u) == 0x1C00u) {
        /** Operation */
        unsigned char d = (instruction >> 4) & 0x1Fu;
        unsigned char r = ((instruction >> 5) & 0x10u) + (instruction & 0xFu);

        unsigned char Rd = SRAM[d];
        unsigned char Rr = SRAM[r];
        unsigned char R  = SRAM[d] = SRAM[Rd] + SRAM[Rr] + BIT_GET(SRAM[0x5Fu], 0);

        /* Setting SREG */
        unsigned char CF, ZF, NF, VF, SF, HF, TF, IF;
        CF = (BIT_GET(Rd, 7) & BIT_GET(Rr, 7)) | (BIT_GET(Rr, 7) & BIT_GET_INV(R, 7)) | (BIT_GET_INV(R, 7) & BIT_GET(Rd, 7));
        ZF = (R == 0) ? 1u : 0u;
        NF = BIT_GET(R, 7);
        VF = (BIT_GET(Rd, 7) & BIT_GET(Rr, 7) & BIT_GET_INV(R, 7)) | (BIT_GET_INV(Rd, 7) & BIT_GET_INV(Rr, 7) & BIT_GET(R, 7));
        SF = NF ^ VF;
        HF = (BIT_GET(Rd, 3) & BIT_GET(Rr, 3)) | (BIT_GET(Rr, 3) & BIT_GET_INV(R, 3)) | (BIT_GET_INV(R, 3) & BIT_GET(Rd, 3));
        TF = BIT_GET(SRAM[0x5Fu], 6);
        IF = BIT_GET(SRAM[0x5Fu], 7);

        SRAM[0x5Fu] = CF + (ZF << 1) + (NF << 2) + (VF << 3) + (SF << 4) + (HF << 5) + (TF << 6) + (IF << 7);

        /** Logging */
        char message[500];
        snprintf(message, sizeof(message), "ADC: PC=0x%04hx, r=%hhu, d=%hhu, Rr=%hhu, Rd=%hhu, R=%hhu, SREG=0x%hhx\n", PC, r, d, Rr, Rd, R, SRAM[0x5Fu]);
        log(ELogMode::All, message);

        /** PC */
        ++PC;

        return;
    }

    /** NOP: 0000 0000 0000 0000 */
    if (instruction == 0) {
        /** Logging */
        char message[500];
        snprintf(message, sizeof(message), "NOP: PC=0x%04hx\n", PC);
        log(ELogMode::All, message);

        /** PC */
        ++PC;

        return;
    }

    /** ...a lot of other operations... */

    /** Unknown instruction */
    char message[500];
    snprintf(message, sizeof(message), "Invalid instruction: PC=0x%04hx\n", PC);
    log(ELogMode::Errors, message);

    Params.lifetime = -1;
}
