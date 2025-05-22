#include <stdio.h>

#include "../Headers/ErrorParser.h"

void ParseError(enum HashErrors code)
{
    switch(code)
    {
        case ZERO_BUCKET_COUNT:
        {
            fprintf(stderr, "Bucket count was set to zero, empty hash table is percieved as an error\n");
            break;
        }
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
            fprintf(stderr, "Failed to allocate memory: \n");
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
        case KEY_IS_TOO_BIG:
        {
            fprintf(stderr, "Key you tried to insert is too big, it must be under 32 bytes.\n");
            break;
        }
        case NO_KEY_TO_DELETE:
        {
            fprintf(stderr, "There is no key to delete\n");
            break;
        }
        default:
        {
            return ;
        }
    }
}