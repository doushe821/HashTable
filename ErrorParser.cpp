#include <stdio.h>

#include "ErrorParser.h"

void ParseError(enum ErrorCodes code)
{
    switch(code)
    {
        case FILE_NULL_POINTER:
        {
            fprintf(stderr, "Failed to open or create file.\n");
            break;
        }
        case BUFFER_NULL_POINTER:
        {
            fprintf(stderr, "Buffer allocation failure.\n");
            break;
        }
        case FREAD_ERROR:
        {
            fprintf(stderr, "Failed to read from file\n");
            break;
        }
        case ALLOCATION_FAILURE:
        {
            fprintf(stderr, "Failed to allocate memory: \n"); // TODO LINE + FILE EVERYWHERE
            break;
        }
        case MODULE_SUCCESS:
        {
            break;
        }
        default:
        {
            return ;
        }
    }
}