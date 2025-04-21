#ifndef HASH_TABLE_H_INCLUDED
#define HASH_TABLE_H_INCLUDED

#include "List/List.h"
#include "ErrorParser.h"

struct HashTable_t 
{
    List_t** KeysBucketArray;
    List_t** ValuesBucketArray;
    enum ErrorCodes ErrorCode;
};

HashTable_t HashTableInit(FILE* fp);
int HashTableDestr(HashTable_t* HashTable);
enum ErrorCodes HashTableSearch(HashTable_t* HashTable, void* Key);
enum ErrorCodes HashTableDump(HashTable_t* HashTable);
ErrorCodes HashTableInsert(const char* key, HashTable_t* HashTable);
size_t SimpleHash(void* value, size_t size);

extern "C" size_t ListSearch(const char* Key, void* listData, size_t ListSize);

#endif