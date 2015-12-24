# ATtiny13A microcontroller emulator
This repository contains ATtiny13A microcontroller emulator source code. It is capable of executing programs prepared by avra and avr-gcc compilers. For detailed info on emulator installation and usage see the end of this page <br>

# Supported instructions
ADD, ADC, ADW, SUB, SUBI, SBC, SBCI, SBIW, AND, ANDI, OR, ORI, EOR, COM, NEG, SBR, CBR, INC, DEC, TST, CLR, SER, MUL, MULS, MULSU, RJMP, IJMP, JMP, RCALL, ICALL, CALL, RET, RETI, CPSE, CP, CPC, CPI, SBRC, SBRS, SBIC, SBIS, BRBS, BRBC, BREQ, BRNE, BRCS, BRCC, BRSH, BRLO, BRMI, BRPL, BRGE, BRLT, BRHS, BRHC, BRTS, BRTC, BRVS, BRVC, BRIE, BRID, MOV, MOVW, LDI, LDS, LD (all variants), LDD, STS, ST (all variants), STD, LPM, IN, OUT, PUSH, POP, XCH, LAS, LAC, LAT, LSL, LSR, ROL, ROR, ASR, SWAP, BSET, BCLR, SBI, CBI, BST, BLD, SEC, CLC, SEN, CLN, SEZ, CLZ, SEI, CLI, SES, CLS, SEV, CLV, SET, CLT, SEH, CLH, NOP, SLEEP <br> <br>
Note that several instructions may appear by a different name in log due to the same opcode of some instructions <br>

# Realized criteria
1) Emulator is able to execute arithmetic and control instructions <br>
2) avra compliled programs are supported <br>
3) Contents of EEPROM can be read <br>
4) Contents of EEPROM can be modified <br>

# Installation
<ol>
  <li>Make sure cmake 3.0 or higher is installed on your computer </li>
  <li>Download the source code and open the bin folder in terminal </li>
  <li>cmake ../ </li>
  <li>make </li>
  <li>Copy emulator to prefered location </li>
</ol>

# Usage
To get futher instructions you can use ./bin/emulator --help <br>

### Example
./bin/emulator ./test/1.hex -lifetime 1 -logall -logfile log.txt
