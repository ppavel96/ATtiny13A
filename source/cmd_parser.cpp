#include <iostream>
#include <iomanip>

#include "cmd_parser.h"

CmdOption::CmdOption(const std::string &InName, const std::string &InHelp, bool InHasArg) :
	Name(InName), Help(InHelp), HasArg(InHasArg) {}

const std::string& CmdOption::GetName() const {
	return Name;
}

const std::string& CmdOption::CmdOption::GetArgAsString() const {
	return Arg;
}

int CmdOption::GetArgAsInt() const {
	try {
		size_t idx;
		int res = std::stoi(Arg, &idx);
		if (idx < Arg.size())
			throw std::invalid_argument(Arg.c_str());
		return res;
	} catch (const std::invalid_argument&) {
		std::cout << "emulator: fatal error: " << Arg << " is not a number\n";
        exit(0);
	} catch (const std::out_of_range&) {
		std::cout << "emulator: fatal error: " << Arg << " is too big\n";
        exit(0);
	}
}

const std::string& CmdOption::GetHelp() const {
	return Help;
}

CmdParser::CmdParser(const std::vector<CmdOption> &InOptions, int argc, char** argv) : Options(InOptions), Current(0) {
	for (int i = 1; i < argc; ++i)
		CmdArgs.push_back(std::string(argv[i]));
}

bool CmdParser::GetNext(CmdOption& OutOption) {
	if (Current == CmdArgs.size())
		return false;
	
	bool Flag = false;
	for (auto i : Options)
		if (i.GetName() == CmdArgs[Current]) {
			OutOption = i;
			Flag = true;
			break;
		}

	if (!Flag) {
		if (CmdArgs[Current][0] == '-') {
			std::cout << "emulator: fatal error: unrecognized command line option " << CmdArgs[Current] << "\n";
            exit(0);
		}

		OutOption.Name = CmdArgs[Current++];
		return true;
	}

	if (OutOption.HasArg) {
		if (++Current == CmdArgs.size()) {
			std::cout << "emulator: fatal error: missing argument after " << OutOption.GetName() << "\n";
            exit(0);
		}

		OutOption.Arg = CmdArgs[Current];
	}

	++Current;
	return true;
}

void CmdParser::DisplayHelp() const {
	std::cout << "Usage: emulator [options] flash_file...\n";
	std::cout << "Options:\n";

	for (auto i : Options)
		std::cout << i.GetHelp() << '\n';
	
	std::cout << "\n";
    std::cout << "Please note that i/o and any logs are disabled by default\n";
}
