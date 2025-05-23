#ifndef CMD_PARSER_H_INCLUDED
#define CMD_PARSER_H_INCLUDED

#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>

#include "ErrorParser.h"

__attribute((unused)) static const int NUMBER_OF_COMMANDS = 1;

enum SearchOptimizations
{
    NO_OPTIMIZATIONS,
    SIMD_HASH,
    SIMD_HASH_STRCMP_IN_ASM,
    SIMD_HASH_ASM_SEARCH,
};

struct flags_t
{
    int searchOptimization;
};

static const option options[NUMBER_OF_COMMANDS]
{
    {"opt", required_argument, NULL, 's'},
};

int ParseCMD(int argc, char** argv, flags_t* flags);

__attribute((unused)) static const char* UNKNOWN_COMMAND_MESSAGE = "Unknown option, it will be ignored\n";
__attribute((unused)) static const char* MISSING_ARGUMENT = "Option missing argument, it will be set to default or ignored\n";
__attribute((unused)) static const char* PARSER_FAILURE = "Command line options parser failed\n";

#endif