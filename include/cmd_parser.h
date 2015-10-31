#pragma once

#include <string>
#include <vector>

class CmdParser;

/** Command line option */
class CmdOption {
    std::string Name;
    std::string Help;
    std::string Arg;
    bool HasArg;

public:
    /** Creates new command line option */
    CmdOption(const std::string &InName, const std::string &InHelp, bool InHasArg);

    /** Returns name */
    const std::string& GetName() const;

    /** Returns argument as string */
    const std::string& GetArgAsString() const;

    /** Returns argument as int */
    int GetArgAsInt() const;

    /** Returns help */
    const std::string& GetHelp() const;

    friend CmdParser;
};

/** Class responsible for parsing command line arguments */
class CmdParser {
    std::vector<CmdOption> Options;
    std::vector<std::string> CmdArgs;
    size_t Current;

public:
    /** Initialize CmdParser with a set of options and ptr to argv */
    CmdParser(const std::vector<CmdOption> &InOptions, int argc, char** argv);

    /** Retrieves the next option and returns true on success */
    bool GetNext(CmdOption& OutOption);

    /** Displays help */
    void DisplayHelp() const;
};
