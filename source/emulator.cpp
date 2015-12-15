#include <cstdlib>
#include "emulator.h"

#define BIT_GET(a,b)     (((a) >> (b)) & 1u)
#define BIT_GET_INV(a,b) ((((a) >> (b)) & 1u) ^ 1u)
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

Instruction::Instruction(std::string InName, std::string InOpcode, uint16_t InSize, void (*InFunc)(class Emulator &ATtiny13A)) :
    Name(InName), Opcode(InOpcode), Size(InSize), Func(InFunc) {}

bool Instruction::TryExecute(class Emulator &ATtiny13A) {
    // Size is 2 words but only one left
    if (ATtiny13A.PC + Size > ATtiny13A.FlashMemory.size())
        return false;

    // Opcode check
    for (uint32_t i = 0; i < 16; ++i)
        if ((Opcode[i] == '0' && BIT_GET(ATtiny13A.FlashMemory[ATtiny13A.PC], 15 - i) != 0) ||
            (Opcode[i] == '1' && BIT_GET(ATtiny13A.FlashMemory[ATtiny13A.PC], 15 - i) != 1))
            return false;

    // Skipping due to conditional branch
    if (ATtiny13A.Skip) {
        ATtiny13A.Skip = false;
        ATtiny13A.PC += Size;
        return true;
    }

    // When logging PC, unchanged version will be used
    uint16_t PCLog = ATtiny13A.PC;

    Func(ATtiny13A);

    // Log
    if (ATtiny13A.Params.LogMode.Detailed)
        fprintf(ATtiny13A.Params.logfile, "%-4hu " BYTETOBINARYPATTERN BYTETOBINARYPATTERN " %-11s %-3hhu %-3hhu %-3hhu %-3hhu %-3hhu %-3hhu %-3hhu %-3hhu %-3hhu %-3hhu %-3hhu %-3hhu %-3hhu %-3hhu %-3hhu %-3hhu"
        " %-3hhu %-3hhu %-3hhu %-3hhu %-3hhu %-3hhu %-3hhu %-3hhu %-3hhu %-3hhu %-3hhu %-3hhu %-3hhu %-3hhu %-3hhu %-3hhu %-3hhu " BYTETOBINARYPATTERN "\n",
        PCLog, BYTETOBINARY(ATtiny13A.FlashMemory[PCLog] >> 8), BYTETOBINARY(ATtiny13A.FlashMemory[PCLog]), Name.c_str(), ATtiny13A.SRAM[0], ATtiny13A.SRAM[1], ATtiny13A.SRAM[2], ATtiny13A.SRAM[3],
        ATtiny13A.SRAM[4], ATtiny13A.SRAM[5], ATtiny13A.SRAM[6], ATtiny13A.SRAM[7], ATtiny13A.SRAM[8], ATtiny13A.SRAM[9], ATtiny13A.SRAM[10], ATtiny13A.SRAM[11], ATtiny13A.SRAM[12], ATtiny13A.SRAM[13],
        ATtiny13A.SRAM[14], ATtiny13A.SRAM[15], ATtiny13A.SRAM[16], ATtiny13A.SRAM[17], ATtiny13A.SRAM[18], ATtiny13A.SRAM[19], ATtiny13A.SRAM[20], ATtiny13A.SRAM[21], ATtiny13A.SRAM[22], ATtiny13A.SRAM[23],
        ATtiny13A.SRAM[24], ATtiny13A.SRAM[25], ATtiny13A.SRAM[26], ATtiny13A.SRAM[27], ATtiny13A.SRAM[28], ATtiny13A.SRAM[29], ATtiny13A.SRAM[30], ATtiny13A.SRAM[31], ATtiny13A.SRAM[0x5D], BYTETOBINARY(ATtiny13A.SRAM[0x5F]));

    return true;
}

Emulator::Emulator(const std::vector<uint16_t> &InFlashMemory, const std::vector<uint8_t> &InEEPROM, const RunParams& InParams) :
    FlashMemory(InFlashMemory), EEPROM(InEEPROM), SRAM(std::vector<uint8_t>(160)), PC(0), Params(InParams), Skip(false) {
    // First log line
    if (Params.LogMode.Detailed)
        fprintf(Params.logfile, "PC   Opcode           Instruction R0  R1  R2  R3  R4  R5  R6  R7  R8  R9  R10 R11 R12 R13 R14 R15 R16 R17 R18 R19 R20 R21 R22 R23 R24 R25 R26 R27 R28 R29 R30 R31 SPL SREG    \n");

    // ---------------------------------------------------------------------------------
    // Configure instructions
    // ---------------------------------------------------------------------------------
    // Arithmetic and Logic Instructions
    // ---------------------------------------------------------------------------------

    InstructionSet.push_back(Instruction("ADD", "000011rdddddrrrr", 1, [](Emulator &ATtiny13A) -> void {
        uint16_t instruction = ATtiny13A.FlashMemory[ATtiny13A.PC];

        /** Operation */
        uint8_t d = (instruction >> 4) & 0x1Fu;
        uint8_t r = ((instruction >> 5) & 0x10u) + (instruction & 0xFu);

        uint8_t Rd = ATtiny13A.SRAM[d];
        uint8_t Rr = ATtiny13A.SRAM[r];
        uint8_t R  = ATtiny13A.SRAM[d] = Rd + Rr;

        /* Setting SREG */
        uint8_t CF, ZF, NF, VF, SF, HF, TF, IF;
        CF = (BIT_GET(Rd, 7) & BIT_GET(Rr, 7)) | (BIT_GET(Rr, 7) & BIT_GET_INV(R, 7)) | (BIT_GET_INV(R, 7) & BIT_GET(Rd, 7));
        ZF = (R == 0) ? 1u : 0u;
        NF = BIT_GET(R, 7);
        VF = (BIT_GET(Rd, 7) & BIT_GET(Rr, 7) & BIT_GET_INV(R, 7)) | (BIT_GET_INV(Rd, 7) & BIT_GET_INV(Rr, 7) & BIT_GET(R, 7));
        SF = NF ^ VF;
        HF = (BIT_GET(Rd, 3) & BIT_GET(Rr, 3)) | (BIT_GET(Rr, 3) & BIT_GET_INV(R, 3)) | (BIT_GET_INV(R, 3) & BIT_GET(Rd, 3));
        TF = BIT_GET(ATtiny13A.SRAM[0x5Fu], 6);
        IF = BIT_GET(ATtiny13A.SRAM[0x5Fu], 7);

        ATtiny13A.SRAM[0x5Fu] = CF + (ZF << 1) + (NF << 2) + (VF << 3) + (SF << 4) + (HF << 5) + (TF << 6) + (IF << 7);

        /** PC */
        ++ATtiny13A.PC;
    }));

    InstructionSet.push_back(Instruction("ADC", "000111rdddddrrrr", 1, [](Emulator &ATtiny13A) -> void {
        uint16_t instruction = ATtiny13A.FlashMemory[ATtiny13A.PC];

        /** Operation */
        uint8_t d = (instruction >> 4) & 0x1Fu;
        uint8_t r = ((instruction >> 5) & 0x10u) + (instruction & 0xFu);

        uint8_t Rd = ATtiny13A.SRAM[d];
        uint8_t Rr = ATtiny13A.SRAM[r];
        uint8_t R  = ATtiny13A.SRAM[d] = Rd + Rr + BIT_GET(ATtiny13A.SRAM[0x5Fu], 0);

        /* Setting SREG */
        uint8_t CF, ZF, NF, VF, SF, HF, TF, IF;
        CF = (BIT_GET(Rd, 7) & BIT_GET(Rr, 7)) | (BIT_GET(Rr, 7) & BIT_GET_INV(R, 7)) | (BIT_GET_INV(R, 7) & BIT_GET(Rd, 7));
        ZF = (R == 0) ? 1u : 0u;
        NF = BIT_GET(R, 7);
        VF = (BIT_GET(Rd, 7) & BIT_GET(Rr, 7) & BIT_GET_INV(R, 7)) | (BIT_GET_INV(Rd, 7) & BIT_GET_INV(Rr, 7) & BIT_GET(R, 7));
        SF = NF ^ VF;
        HF = (BIT_GET(Rd, 3) & BIT_GET(Rr, 3)) | (BIT_GET(Rr, 3) & BIT_GET_INV(R, 3)) | (BIT_GET_INV(R, 3) & BIT_GET(Rd, 3));
        TF = BIT_GET(ATtiny13A.SRAM[0x5Fu], 6);
        IF = BIT_GET(ATtiny13A.SRAM[0x5Fu], 7);

        ATtiny13A.SRAM[0x5Fu] = CF + (ZF << 1) + (NF << 2) + (VF << 3) + (SF << 4) + (HF << 5) + (TF << 6) + (IF << 7);

        /** PC */
        ++ATtiny13A.PC;
    }));

    InstructionSet.push_back(Instruction("ADIW", "10010110KKddKKKK", 1, [](Emulator &ATtiny13A) -> void {
        uint16_t instruction = ATtiny13A.FlashMemory[ATtiny13A.PC];

        /** Operation */
        uint8_t d = (instruction >> 4) & 0x3u;
        uint8_t K = ((instruction >> 2) & 0x30u) + (instruction & 0xFu);

        uint16_t Rdh = ATtiny13A.SRAM[d * 2 + 25];
        uint16_t Rdl = ATtiny13A.SRAM[d * 2 + 24];
        uint16_t Rd = (Rdh << 8) | Rdl;
        uint16_t R = Rd + K;

        ATtiny13A.SRAM[d * 2 + 25] = R >> 8;
        ATtiny13A.SRAM[d * 2 + 24] = R;

        /* Setting SREG */
        uint8_t CF, ZF, NF, VF, SF, HF, TF, IF;
        CF = BIT_GET(Rdh, 7) & BIT_GET_INV(R, 15);
        ZF = (R == 0) ? 1u : 0u;
        NF = BIT_GET(R, 15);
        VF = BIT_GET_INV(Rdh, 7) & BIT_GET(R, 15);
        SF = NF ^ VF;
        HF = BIT_GET(ATtiny13A.SRAM[0x5Fu], 5);
        TF = BIT_GET(ATtiny13A.SRAM[0x5Fu], 6);
        IF = BIT_GET(ATtiny13A.SRAM[0x5Fu], 7);

        ATtiny13A.SRAM[0x5Fu] = CF + (ZF << 1) + (NF << 2) + (VF << 3) + (SF << 4) + (HF << 5) + (TF << 6) + (IF << 7);

        /** PC */
        ++ATtiny13A.PC;
    }));

    InstructionSet.push_back(Instruction("SUB", "000110rdddddrrrr", 1, [](Emulator &ATtiny13A) -> void {
        uint16_t instruction = ATtiny13A.FlashMemory[ATtiny13A.PC];

        /** Operation */
        uint8_t d = (instruction >> 4) & 0x1Fu;
        uint8_t r = ((instruction >> 5) & 0x10u) + (instruction & 0xFu);

        uint8_t Rd = ATtiny13A.SRAM[d];
        uint8_t Rr = ATtiny13A.SRAM[r];
        uint8_t R  = ATtiny13A.SRAM[d] = Rd - Rr;

        /* Setting SREG */
        uint8_t CF, ZF, NF, VF, SF, HF, TF, IF;
        CF = (BIT_GET_INV(Rd, 7) & BIT_GET(Rr, 7)) | (BIT_GET(Rr, 7) & BIT_GET(R, 7)) | (BIT_GET(R, 7) & BIT_GET_INV(Rd, 7));
        ZF = (R == 0) ? 1u : 0u;
        NF = BIT_GET(R, 7);
        VF = (BIT_GET(Rd, 7) & BIT_GET_INV(Rr, 7) & BIT_GET_INV(R, 7)) | (BIT_GET_INV(Rd, 7) & BIT_GET(Rr, 7) & BIT_GET(R, 7));
        SF = NF ^ VF;
        HF = (BIT_GET_INV(Rd, 3) & BIT_GET(Rr, 3)) | (BIT_GET(Rr, 3) & BIT_GET(R, 3)) | (BIT_GET(R, 3) & BIT_GET_INV(Rd, 3));
        TF = BIT_GET(ATtiny13A.SRAM[0x5Fu], 6);
        IF = BIT_GET(ATtiny13A.SRAM[0x5Fu], 7);

        ATtiny13A.SRAM[0x5Fu] = CF + (ZF << 1) + (NF << 2) + (VF << 3) + (SF << 4) + (HF << 5) + (TF << 6) + (IF << 7);

        /** PC */
        ++ATtiny13A.PC;
    }));

    InstructionSet.push_back(Instruction("SUBI", "0101KKKKddddKKKK", 1, [](Emulator &ATtiny13A) -> void {
        uint16_t instruction = ATtiny13A.FlashMemory[ATtiny13A.PC];

        /** Operation */
        uint8_t d = ((instruction >> 4) & 0xFu) + 16;
        uint8_t K = ((instruction >> 4) & 0xF0u) + (instruction & 0xFu);

        uint8_t Rd = ATtiny13A.SRAM[d];
        uint8_t R  = ATtiny13A.SRAM[d] = Rd - K;

        /* Setting SREG */
        uint8_t CF, ZF, NF, VF, SF, HF, TF, IF;
        CF = (BIT_GET_INV(Rd, 7) & BIT_GET(K, 7)) | (BIT_GET(K, 7) & BIT_GET(R, 7)) | (BIT_GET(R, 7) & BIT_GET_INV(Rd, 7));
        ZF = (R == 0) ? 1u : 0u;
        NF = BIT_GET(R, 7);
        VF = (BIT_GET(Rd, 7) & BIT_GET_INV(K, 7) & BIT_GET_INV(R, 7)) | (BIT_GET_INV(Rd, 7) & BIT_GET(K, 7) & BIT_GET(R, 7));
        SF = NF ^ VF;
        HF = (BIT_GET_INV(Rd, 3) & BIT_GET(K, 3)) | (BIT_GET(K, 3) & BIT_GET(R, 3)) | (BIT_GET(R, 3) & BIT_GET_INV(Rd, 3));
        TF = BIT_GET(ATtiny13A.SRAM[0x5Fu], 6);
        IF = BIT_GET(ATtiny13A.SRAM[0x5Fu], 7);

        ATtiny13A.SRAM[0x5Fu] = CF + (ZF << 1) + (NF << 2) + (VF << 3) + (SF << 4) + (HF << 5) + (TF << 6) + (IF << 7);

        /** PC */
        ++ATtiny13A.PC;
    }));

    InstructionSet.push_back(Instruction("SUBC", "000010rdddddrrrr", 1, [](Emulator &ATtiny13A) -> void {
        uint16_t instruction = ATtiny13A.FlashMemory[ATtiny13A.PC];

        /** Operation */
        uint8_t d = (instruction >> 4) & 0x1Fu;
        uint8_t r = ((instruction >> 5) & 0x10u) + (instruction & 0xFu);

        uint8_t Rd = ATtiny13A.SRAM[d];
        uint8_t Rr = ATtiny13A.SRAM[r];
        uint8_t R  = ATtiny13A.SRAM[d] = Rd - Rr - BIT_GET(ATtiny13A.SRAM[0x5Fu], 0);

        /* Setting SREG */
        uint8_t CF, ZF, NF, VF, SF, HF, TF, IF;
        CF = (BIT_GET_INV(Rd, 7) & BIT_GET(Rr, 7)) | (BIT_GET(Rr, 7) & BIT_GET(R, 7)) | (BIT_GET(R, 7) & BIT_GET_INV(Rd, 7));
        ZF = (R == 0) ? 1u : 0u;
        NF = BIT_GET(R, 7);
        VF = (BIT_GET(Rd, 7) & BIT_GET_INV(Rr, 7) & BIT_GET_INV(R, 7)) | (BIT_GET_INV(Rd, 7) & BIT_GET(Rr, 7) & BIT_GET(R, 7));
        SF = NF ^ VF;
        HF = (BIT_GET_INV(Rd, 3) & BIT_GET(Rr, 3)) | (BIT_GET(Rr, 3) & BIT_GET(R, 3)) | (BIT_GET(R, 3) & BIT_GET_INV(Rd, 3));
        TF = BIT_GET(ATtiny13A.SRAM[0x5Fu], 6);
        IF = BIT_GET(ATtiny13A.SRAM[0x5Fu], 7);

        ATtiny13A.SRAM[0x5Fu] = CF + (ZF << 1) + (NF << 2) + (VF << 3) + (SF << 4) + (HF << 5) + (TF << 6) + (IF << 7);

        /** PC */
        ++ATtiny13A.PC;
    }));

    InstructionSet.push_back(Instruction("SBCI", "0100KKKKddddKKKK", 1, [](Emulator &ATtiny13A) -> void {
        uint16_t instruction = ATtiny13A.FlashMemory[ATtiny13A.PC];

        /** Operation */
        uint8_t d = ((instruction >> 4) & 0xFu) + 16;
        uint8_t K = ((instruction >> 4) & 0xF0u) + (instruction & 0xFu);

        uint8_t Rd = ATtiny13A.SRAM[d];
        uint8_t R  = ATtiny13A.SRAM[d] = Rd - K - BIT_GET(ATtiny13A.SRAM[0x5Fu], 0);

        /* Setting SREG */
        uint8_t CF, ZF, NF, VF, SF, HF, TF, IF;
        CF = (BIT_GET_INV(Rd, 7) & BIT_GET(K, 7)) | (BIT_GET(K, 7) & BIT_GET(R, 7)) | (BIT_GET(R, 7) & BIT_GET_INV(Rd, 7));
        ZF = (R == 0) ? 1u : 0u;
        NF = BIT_GET(R, 7);
        VF = (BIT_GET(Rd, 7) & BIT_GET_INV(K, 7) & BIT_GET_INV(R, 7)) | (BIT_GET_INV(Rd, 7) & BIT_GET(K, 7) & BIT_GET(R, 7));
        SF = NF ^ VF;
        HF = (BIT_GET_INV(Rd, 3) & BIT_GET(K, 3)) | (BIT_GET(K, 3) & BIT_GET(R, 3)) | (BIT_GET(R, 3) & BIT_GET_INV(Rd, 3));
        TF = BIT_GET(ATtiny13A.SRAM[0x5Fu], 6);
        IF = BIT_GET(ATtiny13A.SRAM[0x5Fu], 7);

        ATtiny13A.SRAM[0x5Fu] = CF + (ZF << 1) + (NF << 2) + (VF << 3) + (SF << 4) + (HF << 5) + (TF << 6) + (IF << 7);

        /** PC */
        ++ATtiny13A.PC;
    }));

    InstructionSet.push_back(Instruction("SBIW", "10010111KKddKKKK", 1, [](Emulator &ATtiny13A) -> void {
        uint16_t instruction = ATtiny13A.FlashMemory[ATtiny13A.PC];

        /** Operation */
        uint8_t d = (instruction >> 4) & 0x3u;
        uint8_t K = ((instruction >> 2) & 0x30u) + (instruction & 0xFu);

        uint16_t Rdh = ATtiny13A.SRAM[d * 2 + 25];
        uint16_t Rdl = ATtiny13A.SRAM[d * 2 + 24];
        uint16_t Rd = (Rdh << 8) | Rdl;
        uint16_t R = Rd - K;

        ATtiny13A.SRAM[d * 2 + 25] = R >> 8;
        ATtiny13A.SRAM[d * 2 + 24] = R;

        /* Setting SREG */
        uint8_t CF, ZF, NF, VF, SF, HF, TF, IF;
        CF = BIT_GET_INV(Rdh, 7) & BIT_GET(R, 15);
        ZF = (R == 0) ? 1u : 0u;
        NF = BIT_GET(R, 15);
        VF = BIT_GET(Rdh, 7) & BIT_GET_INV(R, 15);
        SF = NF ^ VF;
        HF = BIT_GET(ATtiny13A.SRAM[0x5Fu], 5);
        TF = BIT_GET(ATtiny13A.SRAM[0x5Fu], 6);
        IF = BIT_GET(ATtiny13A.SRAM[0x5Fu], 7);

        ATtiny13A.SRAM[0x5Fu] = CF + (ZF << 1) + (NF << 2) + (VF << 3) + (SF << 4) + (HF << 5) + (TF << 6) + (IF << 7);

        /** PC */
        ++ATtiny13A.PC;
    }));

    InstructionSet.push_back(Instruction("AND", "001000rdddddrrrr", 1, [](Emulator &ATtiny13A) -> void {
        uint16_t instruction = ATtiny13A.FlashMemory[ATtiny13A.PC];

        /** Operation */
        uint8_t d = (instruction >> 4) & 0x1Fu;
        uint8_t r = ((instruction >> 5) & 0x10u) + (instruction & 0xFu);

        uint8_t Rd = ATtiny13A.SRAM[d];
        uint8_t Rr = ATtiny13A.SRAM[r];
        uint8_t R  = ATtiny13A.SRAM[d] = Rd & Rr;

        /* Setting SREG */
        uint8_t CF, ZF, NF, VF, SF, HF, TF, IF;
        CF = BIT_GET(ATtiny13A.SRAM[0x5Fu], 0);
        ZF = (R == 0) ? 1u : 0u;
        NF = BIT_GET(R, 7);
        VF = 0;
        SF = NF ^ VF;
        HF = BIT_GET(ATtiny13A.SRAM[0x5Fu], 5);
        TF = BIT_GET(ATtiny13A.SRAM[0x5Fu], 6);
        IF = BIT_GET(ATtiny13A.SRAM[0x5Fu], 7);

        ATtiny13A.SRAM[0x5Fu] = CF + (ZF << 1) + (NF << 2) + (VF << 3) + (SF << 4) + (HF << 5) + (TF << 6) + (IF << 7);

        /** PC */
        ++ATtiny13A.PC;
    }));

    InstructionSet.push_back(Instruction("ANDI", "0111KKKKddddKKKK", 1, [](Emulator &ATtiny13A) -> void {
        uint16_t instruction = ATtiny13A.FlashMemory[ATtiny13A.PC];

        /** Operation */
        uint8_t d = ((instruction >> 4) & 0xFu) + 16;
        uint8_t K = ((instruction >> 4) & 0xF0u) + (instruction & 0xFu);

        uint8_t Rd = ATtiny13A.SRAM[d];
        uint8_t R  = ATtiny13A.SRAM[d] = Rd & K;

        /* Setting SREG */
        uint8_t CF, ZF, NF, VF, SF, HF, TF, IF;
        CF = BIT_GET(ATtiny13A.SRAM[0x5Fu], 0);
        ZF = (R == 0) ? 1u : 0u;
        NF = BIT_GET(R, 7);
        VF = 0;
        SF = NF ^ VF;
        HF = BIT_GET(ATtiny13A.SRAM[0x5Fu], 5);
        TF = BIT_GET(ATtiny13A.SRAM[0x5Fu], 6);
        IF = BIT_GET(ATtiny13A.SRAM[0x5Fu], 7);

        ATtiny13A.SRAM[0x5Fu] = CF + (ZF << 1) + (NF << 2) + (VF << 3) + (SF << 4) + (HF << 5) + (TF << 6) + (IF << 7);

        /** PC */
        ++ATtiny13A.PC;
    }));

    InstructionSet.push_back(Instruction("OR", "001010rdddddrrrr", 1, [](Emulator &ATtiny13A) -> void {
        uint16_t instruction = ATtiny13A.FlashMemory[ATtiny13A.PC];

        /** Operation */
        uint8_t d = (instruction >> 4) & 0x1Fu;
        uint8_t r = ((instruction >> 5) & 0x10u) + (instruction & 0xFu);

        uint8_t Rd = ATtiny13A.SRAM[d];
        uint8_t Rr = ATtiny13A.SRAM[r];
        uint8_t R  = ATtiny13A.SRAM[d] = Rd | Rr;

        /* Setting SREG */
        uint8_t CF, ZF, NF, VF, SF, HF, TF, IF;
        CF = BIT_GET(ATtiny13A.SRAM[0x5Fu], 0);
        ZF = (R == 0) ? 1u : 0u;
        NF = BIT_GET(R, 7);
        VF = 0;
        SF = NF ^ VF;
        HF = BIT_GET(ATtiny13A.SRAM[0x5Fu], 5);
        TF = BIT_GET(ATtiny13A.SRAM[0x5Fu], 6);
        IF = BIT_GET(ATtiny13A.SRAM[0x5Fu], 7);

        ATtiny13A.SRAM[0x5Fu] = CF + (ZF << 1) + (NF << 2) + (VF << 3) + (SF << 4) + (HF << 5) + (TF << 6) + (IF << 7);

        /** PC */
        ++ATtiny13A.PC;
    }));

    InstructionSet.push_back(Instruction("ORI", "0110KKKKddddKKKK", 1, [](Emulator &ATtiny13A) -> void {
        uint16_t instruction = ATtiny13A.FlashMemory[ATtiny13A.PC];

        /** Operation */
        uint8_t d = ((instruction >> 4) & 0xFu) + 16;
        uint8_t K = ((instruction >> 4) & 0xF0u) + (instruction & 0xFu);

        uint8_t Rd = ATtiny13A.SRAM[d];
        uint8_t R  = ATtiny13A.SRAM[d] = Rd | K;

        /* Setting SREG */
        uint8_t CF, ZF, NF, VF, SF, HF, TF, IF;
        CF = BIT_GET(ATtiny13A.SRAM[0x5Fu], 0);
        ZF = (R == 0) ? 1u : 0u;
        NF = BIT_GET(R, 7);
        VF = 0;
        SF = NF ^ VF;
        HF = BIT_GET(ATtiny13A.SRAM[0x5Fu], 5);
        TF = BIT_GET(ATtiny13A.SRAM[0x5Fu], 6);
        IF = BIT_GET(ATtiny13A.SRAM[0x5Fu], 7);

        ATtiny13A.SRAM[0x5Fu] = CF + (ZF << 1) + (NF << 2) + (VF << 3) + (SF << 4) + (HF << 5) + (TF << 6) + (IF << 7);

        /** PC */
        ++ATtiny13A.PC;
    }));

    InstructionSet.push_back(Instruction("EOR", "001001rdddddrrrr", 1, [](Emulator &ATtiny13A) -> void {
        uint16_t instruction = ATtiny13A.FlashMemory[ATtiny13A.PC];

        /** Operation */
        uint8_t d = (instruction >> 4) & 0x1Fu;
        uint8_t r = ((instruction >> 5) & 0x10u) + (instruction & 0xFu);

        uint8_t Rd = ATtiny13A.SRAM[d];
        uint8_t Rr = ATtiny13A.SRAM[r];
        uint8_t R  = ATtiny13A.SRAM[d] = Rd ^ Rr;

        /* Setting SREG */
        uint8_t CF, ZF, NF, VF, SF, HF, TF, IF;
        CF = BIT_GET(ATtiny13A.SRAM[0x5Fu], 0);
        ZF = (R == 0) ? 1u : 0u;
        NF = BIT_GET(R, 7);
        VF = 0;
        SF = NF ^ VF;
        HF = BIT_GET(ATtiny13A.SRAM[0x5Fu], 5);
        TF = BIT_GET(ATtiny13A.SRAM[0x5Fu], 6);
        IF = BIT_GET(ATtiny13A.SRAM[0x5Fu], 7);

        ATtiny13A.SRAM[0x5Fu] = CF + (ZF << 1) + (NF << 2) + (VF << 3) + (SF << 4) + (HF << 5) + (TF << 6) + (IF << 7);

        /** PC */
        ++ATtiny13A.PC;
    }));

    InstructionSet.push_back(Instruction("COM", "1001010ddddd0000", 1, [](Emulator &ATtiny13A) -> void {
        uint16_t instruction = ATtiny13A.FlashMemory[ATtiny13A.PC];

        /** Operation */
        uint8_t d = (instruction >> 4) & 0x1Fu;

        uint8_t Rd = ATtiny13A.SRAM[d];
        uint8_t R  = ATtiny13A.SRAM[d] = 0xFFu - Rd;

        /* Setting SREG */
        uint8_t CF, ZF, NF, VF, SF, HF, TF, IF;
        CF = 1;
        ZF = (R == 0) ? 1u : 0u;
        NF = BIT_GET(R, 7);
        VF = 0;
        SF = NF ^ VF;
        HF = BIT_GET(ATtiny13A.SRAM[0x5Fu], 5);
        TF = BIT_GET(ATtiny13A.SRAM[0x5Fu], 6);
        IF = BIT_GET(ATtiny13A.SRAM[0x5Fu], 7);

        ATtiny13A.SRAM[0x5Fu] = CF + (ZF << 1) + (NF << 2) + (VF << 3) + (SF << 4) + (HF << 5) + (TF << 6) + (IF << 7);

        /** PC */
        ++ATtiny13A.PC;
    }));

    InstructionSet.push_back(Instruction("NEG", "1001010ddddd0001", 1, [](Emulator &ATtiny13A) -> void {
        uint16_t instruction = ATtiny13A.FlashMemory[ATtiny13A.PC];

        /** Operation */
        uint8_t d = (instruction >> 4) & 0x1Fu;

        uint8_t Rd = ATtiny13A.SRAM[d];
        uint8_t R  = ATtiny13A.SRAM[d] = 0x0u - Rd;

        /* Setting SREG */
        uint8_t CF, ZF, NF, VF, SF, HF, TF, IF;
        CF = (R == 0) ? 0u : 1u;
        ZF = (R == 0) ? 1u : 0u;
        NF = BIT_GET(R, 7);
        VF = (R == 0x80) ? 1u : 0u;
        SF = NF ^ VF;
        HF = BIT_GET(R, 3) | BIT_GET_INV(Rd, 3);
        TF = BIT_GET(ATtiny13A.SRAM[0x5Fu], 6);
        IF = BIT_GET(ATtiny13A.SRAM[0x5Fu], 7);

        ATtiny13A.SRAM[0x5Fu] = CF + (ZF << 1) + (NF << 2) + (VF << 3) + (SF << 4) + (HF << 5) + (TF << 6) + (IF << 7);

        /** PC */
        ++ATtiny13A.PC;
    }));

    InstructionSet.push_back(Instruction("INC", "1001010ddddd0011", 1, [](Emulator &ATtiny13A) -> void {
        uint16_t instruction = ATtiny13A.FlashMemory[ATtiny13A.PC];

        /** Operation */
        uint8_t  d = (instruction >> 4) & 0x1Fu;

        uint8_t  Rd = ATtiny13A.SRAM[d];
        uint8_t  R  = ATtiny13A.SRAM[d] = Rd + 1;

        /* Setting SREG */
        uint8_t  CF, ZF, NF, VF, SF, HF, TF, IF;
        CF = BIT_GET(ATtiny13A.SRAM[0x5Fu], 0);
        ZF = (R == 0) ? 1u : 0u;
        NF = BIT_GET(R, 7);
        VF = (R == 0x80) ? 1u : 0u;
        SF = NF ^ VF;
        HF = BIT_GET(ATtiny13A.SRAM[0x5Fu], 5);
        TF = BIT_GET(ATtiny13A.SRAM[0x5Fu], 6);
        IF = BIT_GET(ATtiny13A.SRAM[0x5Fu], 7);

        ATtiny13A.SRAM[0x5Fu] = CF + (ZF << 1) + (NF << 2) + (VF << 3) + (SF << 4) + (HF << 5) + (TF << 6) + (IF << 7);

        /** PC */
        ++ATtiny13A.PC;
    }));

    InstructionSet.push_back(Instruction("DEC", "1001010ddddd1010", 1, [](Emulator &ATtiny13A) -> void {
        uint16_t instruction = ATtiny13A.FlashMemory[ATtiny13A.PC];

        /** Operation */
        uint8_t  d = (instruction >> 4) & 0x1Fu;

        uint8_t  Rd = ATtiny13A.SRAM[d];
        uint8_t  R  = ATtiny13A.SRAM[d] = Rd - 1;

        /* Setting SREG */
        uint8_t  CF, ZF, NF, VF, SF, HF, TF, IF;
        CF = BIT_GET(ATtiny13A.SRAM[0x5Fu], 0);
        ZF = (R == 0) ? 1u : 0u;
        NF = BIT_GET(R, 7);
        VF = (Rd == 0x80) ? 1u : 0u;
        SF = NF ^ VF;
        HF = BIT_GET(ATtiny13A.SRAM[0x5Fu], 5);
        TF = BIT_GET(ATtiny13A.SRAM[0x5Fu], 6);
        IF = BIT_GET(ATtiny13A.SRAM[0x5Fu], 7);

        ATtiny13A.SRAM[0x5Fu] = CF + (ZF << 1) + (NF << 2) + (VF << 3) + (SF << 4) + (HF << 5) + (TF << 6) + (IF << 7);

        /** PC */
        ++ATtiny13A.PC;
    }));

    InstructionSet.push_back(Instruction("MUL", "100111rdddddrrrr", 1, [](Emulator &ATtiny13A) -> void {
        uint16_t instruction = ATtiny13A.FlashMemory[ATtiny13A.PC];

        /** Operation */
        uint8_t d = (instruction >> 4) & 0x1Fu;
        uint8_t r = ((instruction >> 5) & 0x10u) + (instruction & 0xFu);

        uint16_t Rd = ATtiny13A.SRAM[d];
        uint16_t Rr = ATtiny13A.SRAM[r];
        uint16_t R  = Rd * Rr;

        ATtiny13A.SRAM[0] = R;
        ATtiny13A.SRAM[1] = R >> 8;

        /* Setting SREG */
        uint8_t CF, ZF, NF, VF, SF, HF, TF, IF;
        CF = BIT_GET(R, 15);
        ZF = (R == 0) ? 1u : 0u;
        NF = BIT_GET(ATtiny13A.SRAM[0x5Fu], 2);
        VF = BIT_GET(ATtiny13A.SRAM[0x5Fu], 3);
        SF = BIT_GET(ATtiny13A.SRAM[0x5Fu], 4);
        HF = BIT_GET(ATtiny13A.SRAM[0x5Fu], 5);
        TF = BIT_GET(ATtiny13A.SRAM[0x5Fu], 6);
        IF = BIT_GET(ATtiny13A.SRAM[0x5Fu], 7);

        ATtiny13A.SRAM[0x5Fu] = CF + (ZF << 1) + (NF << 2) + (VF << 3) + (SF << 4) + (HF << 5) + (TF << 6) + (IF << 7);

        /** PC */
        ++ATtiny13A.PC;
    }));

    InstructionSet.push_back(Instruction("MULS", "00000010ddddrrrr", 1, [](Emulator &ATtiny13A) -> void {
        uint16_t instruction = ATtiny13A.FlashMemory[ATtiny13A.PC];

        /** Operation */
        uint8_t d = ((instruction >> 4) & 0xFu) + 16;
        uint8_t r = (instruction & 0xFu) + 16;

        int16_t Rd = reinterpret_cast<int8_t &>(ATtiny13A.SRAM[d]);
        int16_t Rr = reinterpret_cast<int8_t &>(ATtiny13A.SRAM[r]);

        int16_t SR = Rd * Rr;
        uint16_t R = reinterpret_cast<uint16_t &>(SR);

        ATtiny13A.SRAM[0] = R;
        ATtiny13A.SRAM[1] = R >> 8;

        /* Setting SREG */
        uint8_t CF, ZF, NF, VF, SF, HF, TF, IF;
        CF = BIT_GET(R, 15);
        ZF = (R == 0) ? 1u : 0u;
        NF = BIT_GET(ATtiny13A.SRAM[0x5Fu], 2);
        VF = BIT_GET(ATtiny13A.SRAM[0x5Fu], 3);
        SF = BIT_GET(ATtiny13A.SRAM[0x5Fu], 4);
        HF = BIT_GET(ATtiny13A.SRAM[0x5Fu], 5);
        TF = BIT_GET(ATtiny13A.SRAM[0x5Fu], 6);
        IF = BIT_GET(ATtiny13A.SRAM[0x5Fu], 7);

        ATtiny13A.SRAM[0x5Fu] = CF + (ZF << 1) + (NF << 2) + (VF << 3) + (SF << 4) + (HF << 5) + (TF << 6) + (IF << 7);

        /** PC */
        ++ATtiny13A.PC;
    }));

    InstructionSet.push_back(Instruction("MULSU", "000000110ddd0rrr", 1, [](Emulator &ATtiny13A) -> void {
        uint16_t instruction = ATtiny13A.FlashMemory[ATtiny13A.PC];

        /** Operation */
        uint8_t d = ((instruction >> 4) & 0xFu) + 16;
        uint8_t r = (instruction & 0xFu) + 16;

        int16_t  Rd = reinterpret_cast<int8_t &>(ATtiny13A.SRAM[d]);
        uint16_t Rr = ATtiny13A.SRAM[r];
        
        int16_t SR = Rd * Rr;
        uint16_t R = reinterpret_cast<uint16_t &>(SR);

        ATtiny13A.SRAM[0] = R;
        ATtiny13A.SRAM[1] = R >> 8;

        /* Setting SREG */
        uint8_t CF, ZF, NF, VF, SF, HF, TF, IF;
        CF = BIT_GET(R, 15);
        ZF = (R == 0) ? 1u : 0u;
        NF = BIT_GET(ATtiny13A.SRAM[0x5Fu], 2);
        VF = BIT_GET(ATtiny13A.SRAM[0x5Fu], 3);
        SF = BIT_GET(ATtiny13A.SRAM[0x5Fu], 4);
        HF = BIT_GET(ATtiny13A.SRAM[0x5Fu], 5);
        TF = BIT_GET(ATtiny13A.SRAM[0x5Fu], 6);
        IF = BIT_GET(ATtiny13A.SRAM[0x5Fu], 7);

        ATtiny13A.SRAM[0x5Fu] = CF + (ZF << 1) + (NF << 2) + (VF << 3) + (SF << 4) + (HF << 5) + (TF << 6) + (IF << 7);

        /** PC */
        ++ATtiny13A.PC;
    }));

    // ---------------------------------------------------------------------------------
    // Branch Instructions
    // ---------------------------------------------------------------------------------

    InstructionSet.push_back(Instruction("RJMP", "1100kkkkkkkkkkkk", 1, [](Emulator &ATtiny13A) -> void {
        uint16_t k = ATtiny13A.FlashMemory[ATtiny13A.PC] & 0x07FFu;
        if (BIT_GET(ATtiny13A.FlashMemory[ATtiny13A.PC], 11) == 1)
            ATtiny13A.PC += k - 0x7FF;
        else
            ATtiny13A.PC += k + 1;
    }));

    InstructionSet.push_back(Instruction("IJMP", "1001010000001001", 1, [](Emulator &ATtiny13A) -> void {
        ATtiny13A.PC = (ATtiny13A.SRAM[31] << 8) | ATtiny13A.SRAM[30];
    }));

    InstructionSet.push_back(Instruction("JMP", "1001010kkkkk110kkkkkkkkkkkkkkkkk", 2, [](Emulator &ATtiny13A) -> void {
        ATtiny13A.PC = ATtiny13A.FlashMemory[ATtiny13A.PC + 1];
    }));

    InstructionSet.push_back(Instruction("RCALL", "1101kkkkkkkkkkkk", 1, [](Emulator &ATtiny13A) -> void {
        /** Stack */
        ATtiny13A.SRAM[ATtiny13A.SRAM[0x5Du] - 1] = ATtiny13A.PC + 1;
        ATtiny13A.SRAM[ATtiny13A.SRAM[0x5Du]] = (ATtiny13A.PC + 1) >> 8;
        ATtiny13A.SRAM[0x5Du] -= 2;

        /** PC */
        uint16_t k = ATtiny13A.FlashMemory[ATtiny13A.PC] & 0x07FFu;
        if (BIT_GET(ATtiny13A.FlashMemory[ATtiny13A.PC], 11) == 1)
            ATtiny13A.PC += k - 0x7FF;
        else
            ATtiny13A.PC += k + 1;
    }));

    InstructionSet.push_back(Instruction("ICALL", "1001010100001001", 1, [](Emulator &ATtiny13A) -> void {
        /** Stack */
        ATtiny13A.SRAM[ATtiny13A.SRAM[0x5Du] - 1] = ATtiny13A.PC + 1;
        ATtiny13A.SRAM[ATtiny13A.SRAM[0x5Du]] = (ATtiny13A.PC + 1) >> 8;
        ATtiny13A.SRAM[0x5Du] -= 2;

        /** PC */
        ATtiny13A.PC = (ATtiny13A.SRAM[31] << 8) | ATtiny13A.SRAM[30];
    }));

    InstructionSet.push_back(Instruction("CALL", "1001010kkkkk111kkkkkkkkkkkkkkkkk", 2, [](Emulator &ATtiny13A) -> void {
        /** Stack */
        ATtiny13A.SRAM[ATtiny13A.SRAM[0x5Du] - 1] = ATtiny13A.PC + 1;
        ATtiny13A.SRAM[ATtiny13A.SRAM[0x5Du]] = (ATtiny13A.PC + 1) >> 8;
        ATtiny13A.SRAM[0x5Du] -= 2;

        /** PC */
        ATtiny13A.PC = ATtiny13A.FlashMemory[ATtiny13A.PC + 1];
    }));

    InstructionSet.push_back(Instruction("RET", "1001010100001000", 1, [](Emulator &ATtiny13A) -> void {
        /** Operation */
        ATtiny13A.SRAM[0x5Du] += 2;
        ATtiny13A.PC = ATtiny13A.SRAM[ATtiny13A.SRAM[0x5Du] - 1];
        ATtiny13A.PC |= ((uint16_t)ATtiny13A.SRAM[ATtiny13A.SRAM[0x5Du]]) << 8;
    }));

    InstructionSet.push_back(Instruction("RETI", "1001010100011000", 1, [](Emulator &ATtiny13A) -> void {
        /** Operation */
        ATtiny13A.SRAM[0x5Du] += 2;
        ATtiny13A.PC = ATtiny13A.SRAM[ATtiny13A.SRAM[0x5Du] - 1];
        ATtiny13A.PC |= ((uint16_t)ATtiny13A.SRAM[ATtiny13A.SRAM[0x5Du]]) << 8;

        /** SREG */
        uint8_t CF, ZF, NF, VF, SF, HF, TF, IF;
        CF = BIT_GET(ATtiny13A.SRAM[0x5Fu], 0);
        ZF = BIT_GET(ATtiny13A.SRAM[0x5Fu], 1);
        NF = BIT_GET(ATtiny13A.SRAM[0x5Fu], 2);
        VF = BIT_GET(ATtiny13A.SRAM[0x5Fu], 3);
        SF = BIT_GET(ATtiny13A.SRAM[0x5Fu], 4);
        HF = BIT_GET(ATtiny13A.SRAM[0x5Fu], 5);
        TF = BIT_GET(ATtiny13A.SRAM[0x5Fu], 6);
        IF = 1;

        ATtiny13A.SRAM[0x5Fu] = CF + (ZF << 1) + (NF << 2) + (VF << 3) + (SF << 4) + (HF << 5) + (TF << 6) + (IF << 7);
    }));

    InstructionSet.push_back(Instruction("CPSE", "000100rdddddrrrr", 1, [](Emulator &ATtiny13A) -> void {
        uint16_t instruction = ATtiny13A.FlashMemory[ATtiny13A.PC];

        /** Operation */
        uint8_t d = (instruction >> 4) & 0x1Fu;
        uint8_t r = ((instruction >> 5) & 0x10u) + (instruction & 0xFu);

        uint8_t Rd = ATtiny13A.SRAM[d];
        uint8_t Rr = ATtiny13A.SRAM[r];
        
        if (Rd == Rr)
            ATtiny13A.Skip = true;

        /** PC */
        ++ATtiny13A.PC;
    }));

    InstructionSet.push_back(Instruction("CP", "000101rdddddrrrr", 1, [](Emulator &ATtiny13A) -> void {
        uint16_t instruction = ATtiny13A.FlashMemory[ATtiny13A.PC];

        /** Operation */
        uint8_t d = (instruction >> 4) & 0x1Fu;
        uint8_t r = ((instruction >> 5) & 0x10u) + (instruction & 0xFu);

        uint8_t Rd = ATtiny13A.SRAM[d];
        uint8_t Rr = ATtiny13A.SRAM[r];
        uint8_t R  = Rd - Rr;

        /* Setting SREG */
        uint8_t CF, ZF, NF, VF, SF, HF, TF, IF;
        CF = (BIT_GET_INV(Rd, 7) & BIT_GET(Rr, 7)) | (BIT_GET(Rr, 7) & BIT_GET(R, 7)) | (BIT_GET(R, 7) & BIT_GET_INV(Rd, 7));
        ZF = (R == 0) ? 1u : 0u;
        NF = BIT_GET(R, 7);
        VF = (BIT_GET(Rd, 7) & BIT_GET_INV(Rr, 7) & BIT_GET_INV(R, 7)) | (BIT_GET_INV(Rd, 7) & BIT_GET(Rr, 7) & BIT_GET(R, 7));
        SF = NF ^ VF;
        HF = (BIT_GET_INV(Rd, 3) & BIT_GET(Rr, 3)) | (BIT_GET(Rr, 3) & BIT_GET(R, 3)) | (BIT_GET(R, 3) & BIT_GET_INV(Rd, 3));
        TF = BIT_GET(ATtiny13A.SRAM[0x5Fu], 6);
        IF = BIT_GET(ATtiny13A.SRAM[0x5Fu], 7);

        ATtiny13A.SRAM[0x5Fu] = CF + (ZF << 1) + (NF << 2) + (VF << 3) + (SF << 4) + (HF << 5) + (TF << 6) + (IF << 7);

        /** PC */
        ++ATtiny13A.PC;
    }));

    InstructionSet.push_back(Instruction("CPC", "000001rdddddrrrr", 1, [](Emulator &ATtiny13A) -> void {
        uint16_t instruction = ATtiny13A.FlashMemory[ATtiny13A.PC];

        /** Operation */
        uint8_t d = (instruction >> 4) & 0x1Fu;
        uint8_t r = ((instruction >> 5) & 0x10u) + (instruction & 0xFu);

        uint8_t Rd = ATtiny13A.SRAM[d];
        uint8_t Rr = ATtiny13A.SRAM[r];
        uint8_t R  = Rd - Rr - BIT_GET(ATtiny13A.SRAM[0x5Fu], 0);

        /* Setting SREG */
        uint8_t CF, ZF, NF, VF, SF, HF, TF, IF;
        CF = (BIT_GET_INV(Rd, 7) & BIT_GET(Rr, 7)) | (BIT_GET(Rr, 7) & BIT_GET(R, 7)) | (BIT_GET(R, 7) & BIT_GET_INV(Rd, 7));
        ZF = (R == 0) ? 1u : 0u;
        NF = BIT_GET(R, 7);
        VF = (BIT_GET(Rd, 7) & BIT_GET_INV(Rr, 7) & BIT_GET_INV(R, 7)) | (BIT_GET_INV(Rd, 7) & BIT_GET(Rr, 7) & BIT_GET(R, 7));
        SF = NF ^ VF;
        HF = (BIT_GET_INV(Rd, 3) & BIT_GET(Rr, 3)) | (BIT_GET(Rr, 3) & BIT_GET(R, 3)) | (BIT_GET(R, 3) & BIT_GET_INV(Rd, 3));
        TF = BIT_GET(ATtiny13A.SRAM[0x5Fu], 6);
        IF = BIT_GET(ATtiny13A.SRAM[0x5Fu], 7);

        ATtiny13A.SRAM[0x5Fu] = CF + (ZF << 1) + (NF << 2) + (VF << 3) + (SF << 4) + (HF << 5) + (TF << 6) + (IF << 7);

        /** PC */
        ++ATtiny13A.PC;
    }));

    InstructionSet.push_back(Instruction("CPI", "0011KKKKddddKKKK", 1, [](Emulator &ATtiny13A) -> void {
        uint16_t instruction = ATtiny13A.FlashMemory[ATtiny13A.PC];

        /** Operation */
        uint8_t d = ((instruction >> 4) & 0xFu) + 16;
        uint8_t K = ((instruction >> 4) & 0xF0u) + (instruction & 0xFu);

        uint8_t Rd = ATtiny13A.SRAM[d];
        uint8_t R  = Rd - K;

        /* Setting SREG */
        uint8_t CF, ZF, NF, VF, SF, HF, TF, IF;
        CF = (BIT_GET_INV(Rd, 7) & BIT_GET(K, 7)) | (BIT_GET(K, 7) & BIT_GET(R, 7)) | (BIT_GET(R, 7) & BIT_GET_INV(Rd, 7));
        ZF = (R == 0) ? 1u : 0u;
        NF = BIT_GET(R, 7);
        VF = (BIT_GET(Rd, 7) & BIT_GET_INV(K, 7) & BIT_GET_INV(R, 7)) | (BIT_GET_INV(Rd, 7) & BIT_GET(K, 7) & BIT_GET(R, 7));
        SF = NF ^ VF;
        HF = (BIT_GET_INV(Rd, 3) & BIT_GET(K, 3)) | (BIT_GET(K, 3) & BIT_GET(R, 3)) | (BIT_GET(R, 3) & BIT_GET_INV(Rd, 3));
        TF = BIT_GET(ATtiny13A.SRAM[0x5Fu], 6);
        IF = BIT_GET(ATtiny13A.SRAM[0x5Fu], 7);

        ATtiny13A.SRAM[0x5Fu] = CF + (ZF << 1) + (NF << 2) + (VF << 3) + (SF << 4) + (HF << 5) + (TF << 6) + (IF << 7);

        /** PC */
        ++ATtiny13A.PC;
    }));

    InstructionSet.push_back(Instruction("SBRC", "1111110rrrrr0bbb", 1, [](Emulator &ATtiny13A) -> void {
        uint16_t instruction = ATtiny13A.FlashMemory[ATtiny13A.PC];

        /** Operation */
        uint8_t r = (instruction >> 4) & 0x1Fu;
        uint8_t b = instruction & 0x7u;

        uint8_t Rr = ATtiny13A.SRAM[r];
        
        if (BIT_GET(Rr, b) == 0)
            ATtiny13A.Skip = true;

        /** PC */
        ++ATtiny13A.PC;
    }));

    InstructionSet.push_back(Instruction("SBRS", "1111111rrrrr0bbb", 1, [](Emulator &ATtiny13A) -> void {
        uint16_t instruction = ATtiny13A.FlashMemory[ATtiny13A.PC];

        /** Operation */
        uint8_t r = (instruction >> 4) & 0x1Fu;
        uint8_t b = instruction & 0x7u;

        uint8_t Rr = ATtiny13A.SRAM[r];
        
        if (BIT_GET(Rr, b) == 1)
            ATtiny13A.Skip = true;

        /** PC */
        ++ATtiny13A.PC;
    }));

    

    // ---------------------------------------------------------------------------------
    // Data Transfer Instructions
    // ---------------------------------------------------------------------------------

    /** MOV: */

    /** MOVW: */

    InstructionSet.push_back(Instruction("LDI", "1110KKKKddddKKKK", 1, [](Emulator &ATtiny13A) -> void {
        uint16_t instruction = ATtiny13A.FlashMemory[ATtiny13A.PC];

        /** Operation */
        uint8_t d = ((instruction >> 4) & 0xFu) + 16;
        uint8_t K = ((instruction >> 4) & 0xF0u) + (instruction & 0xFu);

        ATtiny13A.SRAM[d] = K;

        /** PC */
        ++ATtiny13A.PC;
    }));

    // ---------------------------------------------------------------------------------
    // Bit and Bit-set Instructions
    // ---------------------------------------------------------------------------------

    // ---------------------------------------------------------------------------------
    // MCU Control Instructions
    // ---------------------------------------------------------------------------------

    // It is the last instruction with opcode set to **************** on purpose
    InstructionSet.push_back(Instruction("NOP", "****************", 1, [](Emulator &ATtiny13A) -> void {
        ++ATtiny13A.PC;
    }));
}

bool Emulator::CheckForInterrupt() {
    return false;
}

void Emulator::Run() {
    std::clock_t start_time = std::clock();

    while (true) {
        if (!CheckForInterrupt())
            ProcessInstruction();

        if ((std::clock() - start_time) / (double)(CLOCKS_PER_SEC / 1000) > Params.lifetime)
            return;
    }
}

void Emulator::ProcessInstruction() {
    /** PC overflow */
    if (PC >= FlashMemory.size())
        PC = PC % 512;
    
    for (auto& i : InstructionSet)
        if (i.TryExecute(*this))
            break;
}
