#include <iostream>
#include <cstring>

#include "emulator.h"

const char* help_text = "Usage: emulator [options] flash_file...\n"
                        "Options:\n"
                        "  --help                   Display this information\n"
                        "  -lifetime N              Stop emulator after N ms. Default is 1000\n"
                        "  -eeprom_in <file>        Initialize EEPROM with content of <file>\n"
                        "  -eeprom_out <file>       Save EEPROM to <file> in hex format\n"
                        "  -in <file>               Specify <file> in CSV format as input\n"
                        "  -out <file>              Specify <file> in CSV format as output\n"
                        "  -logfile <file>          Print log to <file>, not to stdout\n"
                        "  -logerr                  Enable error log\n"
                        "  -logwarn                 Enable warnings log\n"
                        "  -logio                   Enable i/o log\n"
                        "  -logint                  Enable interrupt log\n"
                        "  -logall                  Enable log of every instruction\n"
                        "\n"
                        "Please note that i/o and any logs are disabled by default\n";

int main(int argc, char** argv) {
    if (argc >= 2 && strcmp(argv[1], "--help") == 0) {
        std::cout << help_text;
        return 0;
    }

    RunParams DefaultParams;
    char *Flash = 0, *EEPROM_IN = 0, *EEPROM_OUT = 0;

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "-lifetime") == 0) {
            char* end;

            errno = 0;
            DefaultParams.lifetime = std::strtol(argv[++i], &end, 10);
            if (errno != 0 || *end != 0) {
                std::cout << "emulator: fatal error: " << argv[i] << " is not a number\n";
                return 0;
            }

            continue;
        }

        if (strcmp(argv[i], "-eeprom_in") == 0) {
            EEPROM_IN = argv[++i];
            continue;
        }

        if (strcmp(argv[i], "-eeprom_out") == 0) {
            EEPROM_OUT = argv[++i];
            continue;
        }

        if (strcmp(argv[i], "-in") == 0) {
            DefaultParams.infile = fopen(argv[++i], "r");
            if (DefaultParams.infile == nullptr) {
                std::cout << "emulator: fatal error: coudn't open file named " << argv[i] << "\n";
                return 0;
            }

            continue;
        }

        if (strcmp(argv[i], "-out") == 0) {
            DefaultParams.outfile = fopen(argv[++i], "w");
            if (DefaultParams.outfile == nullptr) {
                std::cout << "emulator: fatal error: coudn't open file named " << argv[i] << "\n";
                return 0;
            }

            continue;
        }

        if (strcmp(argv[i], "-logfile") == 0) {
            DefaultParams.logfile = fopen(argv[++i], "w");
            if (DefaultParams.logfile == nullptr) {
                std::cout << "emulator: fatal error: coudn't open file named " << argv[i] << "\n";
                return 0;
            }

            continue;
        }

        if (strcmp(argv[i], "-logerr") == 0) {
            DefaultParams.LogMode = (ELogMode)(DefaultParams.LogMode | ELogMode::Errors);
            continue;
        }

        if (strcmp(argv[i], "-logwarn") == 0) {
            DefaultParams.LogMode = (ELogMode)(DefaultParams.LogMode | ELogMode::Warnings);
            continue;
        }

        if (strcmp(argv[i], "-logio") == 0) {
            DefaultParams.LogMode = (ELogMode)(DefaultParams.LogMode | ELogMode::InOut);
            continue;
        }

        if (strcmp(argv[i], "-logint") == 0) {
            DefaultParams.LogMode = (ELogMode)(DefaultParams.LogMode | ELogMode::Interrupts);
            continue;
        }

        if (strcmp(argv[i], "-logall") == 0) {
            DefaultParams.LogMode = (ELogMode)(DefaultParams.LogMode | ELogMode::All);
            continue;
        }

        if (argv[i][0] == '-') {
            std::cout << "emulator: fatal error: unrecognized command line option " << argv[i] << "\n";
            return 0;
        }
        
        Flash = argv[i];
    }

    if (Flash == 0) {
        std::cout << "emulator: fatal error: no flash file\n";
        return 0;
    }

    // read flash & eeprom
    // create emulator

    std::cout << "emulator: emulation started\n";

    // Emulator.Run();

    std::cout << "emulator: emulation stoped\n";

    // save eeprom
    // close files
}
