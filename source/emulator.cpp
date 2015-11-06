#include <cstdlib>
#include "emulator.h"

#define BIT_GET(a,b)     (((a) >> b) & 1u)
#define BIT_GET_INV(a,b) ((((a) >> b) & 1u) ^ 1u)
#define BYTETOBINARYPATTERN "%d%d%d%d%d%d%d%d"
#define BYTETOBINARY(byte)  \
  (byte & 0x80 ? 1 : 0), \
  (byte & 0x40 ? 1 : 0), \
  (byte & 0x20 ? 1 : 0), \
  (byte & 0x10 ? 1 : 0), \
  (byte & 0x08 ? 1 : 0), \
  (byte & 0x04 ? 1 : 0), \
  (byte & 0x02 ? 1 : 0), \
  (byte & 0x01 ? 1 : 0) 

Emulator::Emulator(const std::vector<unsigned short> &InFlashMemory, const std::vector<unsigned char> &InEEPROM, const RunParams& InParams) :
    FlashMemory(InFlashMemory), EEPROM(InEEPROM), Params(InParams) {
    SRAM = std::vector<unsigned char>(Info.SRAM);
    PC = 0;
}

bool Emulator::CheckForInterrupt() {
    return false;
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
        PC = PC % (Info.FlashMemory / 2);

        if (Params.LogMode & ELogMode::Warnings)
            fprintf(Params.logfile, "PC OVERFLOW WARNING: PC -> 0x%04hx", PC);
    }

    /** Get instruction */
    unsigned short instruction = FlashMemory[PC];

    // ---------------------------------------------------------------------------------
    // Arithmetic and Logic Instructions
    // ---------------------------------------------------------------------------------

    /** ADD: 0000 11rd dddd rrrr */
    if ((instruction & 0xFC00u) == 0x0C00u) {
        /** Operation */
        unsigned char d = (instruction >> 4) & 0x1Fu;
        unsigned char r = ((instruction >> 5) & 0x10u) + (instruction & 0xFu);

        unsigned char Rd = SRAM[d];
        unsigned char Rr = SRAM[r];
        unsigned char R  = SRAM[d] = Rd + Rr;

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
        if (Params.LogMode & ELogMode::All)
            fprintf(Params.logfile, " ADD: PC=0x%04hx, r=%hhu, d=%hhu, Rr=%hhu, Rd=%hhu, R=%hhu, SREG=" BYTETOBINARYPATTERN "\n",
                PC, r, d, Rr, Rd, R, BYTETOBINARY(SRAM[0x5Fu]));

        /** PC */
        ++PC;

        return;
    }
    
    /** ADC: 0001 11rd dddd rrrr */
    if ((instruction & 0xFC00u) == 0x1C00u) {
        /** Operation */
        unsigned char d = (instruction >> 4) & 0x1Fu;
        unsigned char r = ((instruction >> 5) & 0x10u) + (instruction & 0xFu);

        unsigned char Rd = SRAM[d];
        unsigned char Rr = SRAM[r];
        unsigned char R  = SRAM[d] = Rd + Rr + BIT_GET(SRAM[0x5Fu], 0);

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
        if (Params.LogMode & ELogMode::All)
            fprintf(Params.logfile, " ADD: PC=0x%04hx, r=%hhu, d=%hhu, Rr=%hhu, Rd=%hhu, R=%hhu, SREG=" BYTETOBINARYPATTERN "\n",
                PC, r, d, Rr, Rd, R, BYTETOBINARY(SRAM[0x5Fu]));

        /** PC */
        ++PC;

        return;
    }

    /** ADIW: 1001 0110 KKdd KKKK */

    /** SUB: 0001 10rd dddd rrrr */

    /** SUBI: 0101 KKKK dddd KKKK */

    /** SBC: 0000 10rd dddd rrrr */

    /** SBCI: 0100 KKKK dddd KKKK */

    /** SBIW: 1001 0111 KKdd KKKK */

    /** AND: */

    /** ANDI: */

    /** OR: */

    /** ORI: */

    /** EOR: */

    /** COM: */

    /** NEG: */

    /** SBR: */

    /** CBR: */

    /** INC: */

    /** DEC: */

    /** TST: */

    /** CLR: */

    /** SER: */

    /** MUL: */

    /** MULS: */

    /** MULSU: */

    /** FMUL: */

    /** FMULS: */

    /** FMULSU: */

    /** DES: */

    // ---------------------------------------------------------------------------------
    // Branch Instructions
    // ---------------------------------------------------------------------------------

    /** RJMP: 1100 kkkk kkkk kkkk */
    if ((instruction & 0xF000u) == 0xC000u) {
        /** Operation */
        unsigned short k = instruction & 0x07FFu;
        unsigned short NextPC;

        if ((instruction >> 11) & 1u)
            NextPC = PC + k - 0x7FFu;
        else
            NextPC = PC + k + 1u;

        /** Logging */
        if (Params.LogMode & ELogMode::All)
            fprintf(Params.logfile, "RJMP: PC=0x%04hx, NextPC=0x%04hx, k=%hu\n", PC, NextPC, k);

        /** PC */
        PC = NextPC;

        return;
    }

    // ---------------------------------------------------------------------------------
    // Data Transfer Instructions
    // ---------------------------------------------------------------------------------

    /** MOV: */

    /** MOVW: */

    /** LDI: 1110 KKKK dddd KKKK */
    if ((instruction & 0xF000u) == 0xE000u) {
        /** Operation */
        unsigned char d = ((instruction >> 4) & 0xFu) + 16;
        unsigned char K = ((instruction >> 8) & 0xFu) + (instruction & 0xFu);

        SRAM[d] = K;

        /** Logging */
        if (Params.LogMode & ELogMode::All)
            fprintf(Params.logfile, " LDI: PC=0x%04hx, d=%hhu, K=%hhu\n", PC, d, K);

        /** PC */
        ++PC;

        return;
    }

    // ---------------------------------------------------------------------------------
    // Bit and Bit-set Instructions
    // ---------------------------------------------------------------------------------

    // ---------------------------------------------------------------------------------
    // MCU Control Instructions
    // ---------------------------------------------------------------------------------

    /** NOP: 0000 0000 0000 0000 */
    if (instruction == 0) {
        /** Logging */
        if (Params.LogMode & ELogMode::All)
            fprintf(Params.logfile, " NOP: PC=0x%04hx\n", PC);

        /** PC */
        ++PC;

        return;
    }

    /** Unknown instruction */
    if (Params.LogMode & ELogMode::Errors)
        fprintf(Params.logfile, "INVALID INSTRUCTION ERROR: PC=0x%04hx\n", PC);

    Params.lifetime = -1;
}
