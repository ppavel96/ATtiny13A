# ATtiny13A microcontroller emulator
This repository contains ATtiny13A microcontroller emulator source code. <br>
Emulator is capable of executing programs prepared by avra and avr-gcc compilers. <br>
I/O is emulated using CSV tables. <br>

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
