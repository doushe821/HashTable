

#include <string.h>

#include "../Headers/CMDParser.h"

//static char* itoa(int num, char* string);

int ParseCMD(int argc, char** argv, flags_t* flags)
{
    if(argc < 2)
    {
        return MODULE_SUCCESS;
    }

    int opt = 0;
    int optid = 0;
    for(int i = 0; i < argc - 1; i++)
    {
        if((opt = getopt_long(argc, argv, "", options, &optid)) != -1)
        {
            switch(opt)
            {
                case 's':
                {
                    flags->searchOptimization = atoi(optarg);
                    break;
                }
                case '?':
                {
                    fprintf(stderr, "%s", UNKNOWN_COMMAND_MESSAGE);
                    return -1;
                }
                case ':':
                {
                    fprintf(stderr, "%s", MISSING_ARGUMENT);
                    return -1;
                }
                default:
                {
                    fprintf(stderr, "%s", PARSER_FAILURE);
                    return -1;
                }
            }
        }
    }
    return 0;
}


//static char* itoa(int num, char* string)
//{
//    sprintf(string, "%d", num);
//    return string;
//}
