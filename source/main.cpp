#include <iostream>
#include <string>
#include <vector>

#include "emulator.h"
#include "cmd_parser.h"
#include "image_manager.h"

/** Determine if host is little endian */
bool IsLittleEndian() {
    const unsigned short x = 1;
    return *((unsigned char *) &x) == 1;
}

/** Main */
int main(int argc, char** argv) {
    /** Params to get from argv */
    RunParams DefaultParams;
    std::string Flash, EEPROM_IN, EEPROM_OUT;

    /** Configuring possible command line options */
    std::vector<CmdOption> Options;
    Options.push_back(CmdOption("--help",        "  --help                   Display this information",                  false));
    Options.push_back(CmdOption("-lifetime",     "  -lifetime N              Stop emulator after N ms. Default is 1000", true));
    Options.push_back(CmdOption("-eeprom_in",    "  -eeprom_in <file>        Initialize EEPROM with content of <file>",  true));
    Options.push_back(CmdOption("-eeprom_out",   "  -in <file>               Specify <file> in CSV format as input",     true));
    Options.push_back(CmdOption("-in",           "  -out <file>              Specify <file> in CSV format as output",    true));
    Options.push_back(CmdOption("-logfile",      "  -logfile <file>          Print log to <file>, not to stdout",        true));
    Options.push_back(CmdOption("-logerr",       "  -logerr                  Enable error log",                          false));
    Options.push_back(CmdOption("-logwarn",      "  -logwarn                 Enable warnings log",                       false));
    Options.push_back(CmdOption("-logerr",       "  -logio                   Enable i/o log",                            false));
    Options.push_back(CmdOption("-logio",        "  -logint                  Enable interrupt log",                      false));
    Options.push_back(CmdOption("-logint",       "  -logint                  Enable interrupt log",                      false));
    Options.push_back(CmdOption("-logall",       "  -logall                  Enable log of every instruction",           false));

    /** Creating command line parser */
    CmdParser Parser(std::move(Options), argc, argv);
    CmdOption OutOption("", "", false);

    /** Proccessing arguments */
    while (Parser.GetNext(OutOption)) {
        if (OutOption.GetName() == "--help") {
            Parser.DisplayHelp();
            return 0;
        }

        if (OutOption.GetName() == "-lifetime") {
            DefaultParams.lifetime = OutOption.GetArgAsInt();
            continue;
        }

        if (OutOption.GetName() == "-eeprom_in") {
            EEPROM_IN = OutOption.GetArgAsString();
            continue;
        }

        if (OutOption.GetName() == "-eeprom_out") {
            EEPROM_OUT = OutOption.GetArgAsString();
            continue;
        }

        if (OutOption.GetName() == "-in") {
            DefaultParams.infile = fopen(OutOption.GetArgAsString().c_str(), "r");
            if (DefaultParams.infile == nullptr) {
                std::cout << "emulator: fatal error: coudn't open file named " << OutOption.GetArgAsString() << "\n";
                return 0;
            }

            continue;
        }

        if (OutOption.GetName() == "-out") {
            DefaultParams.outfile = fopen(OutOption.GetArgAsString().c_str(), "w");
            if (DefaultParams.infile == nullptr) {
                std::cout << "emulator: fatal error: coudn't open file named " << OutOption.GetArgAsString() << "\n";
                return 0;
            }

            continue;
        }

        if (OutOption.GetName() == "-logfile") {
            DefaultParams.infile = fopen(OutOption.GetArgAsString().c_str(), "w");
            if (DefaultParams.infile == nullptr) {
                std::cout << "emulator: fatal error: coudn't open file named " << OutOption.GetArgAsString() << "\n";
                return 0;
            }

            continue;
        }

        if (OutOption.GetName() == "-logerr") {
            DefaultParams.LogMode = (ELogMode)(DefaultParams.LogMode | ELogMode::Errors);
            continue;
        }

        if (OutOption.GetName() == "-logwarn") {
            DefaultParams.LogMode = (ELogMode)(DefaultParams.LogMode | ELogMode::Warnings);
            continue;
        }

        if (OutOption.GetName() == "-logio") {
            DefaultParams.LogMode = (ELogMode)(DefaultParams.LogMode | ELogMode::InOut);
            continue;
        }

        if (OutOption.GetName() == "-logint") {
            DefaultParams.LogMode = (ELogMode)(DefaultParams.LogMode | ELogMode::Interrupts);
            continue;
        }

        if (OutOption.GetName() == "-logall")  {
            DefaultParams.LogMode = (ELogMode)(DefaultParams.LogMode | ELogMode::All);    
            continue;    
        }
        
        Flash = OutOption.GetName();
    }

    if (Flash.empty()) {
        std::cout << "emulator: fatal error: no flash file\n";
        return 0;
    }

    // Reading flash
    std::vector<unsigned short> FlashMemory(1024);
    ImageManager::ReadImage(Flash.c_str(), (unsigned char*)FlashMemory.data(), FlashMemory.size() * 2);
    if (!IsLittleEndian())
        for (size_t i = 0, sz = FlashMemory.size(); i < sz; ++i)
            FlashMemory[i] = (FlashMemory[i] << 8) + (FlashMemory[i] >> 8);
    
    // Reading EEPROM
    std::vector<unsigned char> EEPROMMemory(64);
    if (!EEPROM_IN.empty())
        ImageManager::ReadImage(EEPROM_IN.c_str(), EEPROMMemory.data(), EEPROMMemory.size());

    // Emulation
    Emulator ATtiny13A(std::move(FlashMemory), std::move(EEPROMMemory), DefaultParams);

    std::cout << "emulator: emulation started\n";
    ATtiny13A.Run();
    std::cout << "emulator: emulation stoped\n";

    // Saving EEPROM
    if (!EEPROM_OUT.empty())
        ImageManager::WriteHexImage(EEPROM_OUT.c_str(), ATtiny13A.GetEEPROM().data(), ATtiny13A.GetEEPROM().size());
}
