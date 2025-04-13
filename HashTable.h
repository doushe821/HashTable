#ifndef HASH_TABLE_H_INCLUDED
#define HASH_TABLE_H_INCLUDED

#include "List/List.h"
#include "ErrorParser.h"

struct HashTable_t 
{
    List_t** BucketArray;
    enum ErrorCodes ErrorCode;
};

HashTable_t HashTableInit(FILE* fp);
int HashTableDestr(HashTable_t* HashTable);
enum ErrorCodes HashTableDump(HashTable_t* HashTable);

#endif