#ifndef ERROR_PARSER_H_INCLUDED
#define ERROR_PARSER_H_INCLUDED

enum HashErrors
{
    FILE_NULL_POINTER = -9,
    NULL_KEY_POINTER,
    NULL_HASH_TABLE_POINTER,
    BUFFER_NULL_POINTER,
    FREAD_ERROR,
    ALLOCATION_FAILURE,
    UNEXPECTED_BEHAVIOUR,
    KEY_IS_TOO_BIG,
    NO_KEY_TO_DELETE,
    MISSALIGNMENT,
    ZERO_BUCKET_COUNT,
    MODULE_SUCCESS = 0,
};

void ParseError(enum HashErrors code);

#endif // ERROR_PARSER_H_INCLUDED