#ifndef HASH_TABLE_H_INCLUDED
#define HASH_TABLE_H_INCLUDED

#include "List/List.h"
#include "ErrorParser.h"

struct HashTable_t 
{
    List_t** KeysBucketArray;
    List_t** ValuesBucketArray;
    size_t BucketCount;
    enum HashErrors ErrorCode;
};

HashTable_t HashTableInit(size_t BucketCount);

int HashTableDestr(HashTable_t* HashTable);

size_t HashTableSearchNaive(HashTable_t* HashTable, void* Key);
size_t HashTableSearchHashOptimized(HashTable_t* HashTable, void* Key);

size_t HashTableSearch(HashTable_t* HashTable, void* Key);

enum HashErrors HashTableDump(HashTable_t* HashTable);

HashErrors HashTableInsert(const char* key, HashTable_t* HashTable);
HashErrors HashTableDelete(const char* key, HashTable_t* HashTable);

size_t SimpleHash(void* value, size_t size);

char* FileToHashTableWordArray(FILE* fp, size_t* WordCounter);

extern "C" size_t ListSearch(const char* Key, void* listData, size_t ListSize);

#endif