# ATtiny13A microcontroller emulator
This repository contains ATtiny13A microcontroller emulator source code. <br>
Emulator is capable of executing programs prepared by avra and avr-gcc compilers. <br>
I/O is emulated using CSV tables. <br>
Now project is in very early development stage and <b>may not work as expected </b><br>

# Installation
### Linux
1. Download the source code
2. Open the source folder in terminal
3. Use make command to compile, binary can be found in ./bin folder
4. Copy ./bin/emulator to prefered location

### Windows
This program hasn't been tested under Windows yet but you can try to
<ol>
  <li>Install MinGW and try to compile using it </li>
  <li>Modify makefile to work with Visual Studio nmake </li>
</ol>

# Usage
To get futher instructions you can use ./bin/emulator --help <br>

### Example
./bin/emulator ./test/1.hex -lifetime 1 -logall -logerr -logfile log.txt
