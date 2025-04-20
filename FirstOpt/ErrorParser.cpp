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
        case NULL_KEY_POINTER:
        {
            fprintf(stderr, "Key pointer is NULL\n");
            break;
        }
        case NULL_HASH_TABLE_POINTER:
        {
            fprintf(stderr, "Hash Table Pointer is NULL\n");
            break;
        }
        case MODULE_SUCCESS:
        {
            break;
        }
        case UNEXPECTED_BEHAVIOUR:
        {
            fprintf(stderr, "Program faced unexpected behaviour on ...\n");
            break;
        }
        default:
        {
            return ;
        }
    }
}