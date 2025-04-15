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
enum ErrorCodes HashTableSearch(HashTable_t* HashTable, void* Key);
enum ErrorCodes HashTableDump(HashTable_t* HashTable);

extern "C" int strcmp64byte(const char* stringA, const char* stringB);

#endif