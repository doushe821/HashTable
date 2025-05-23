#ifndef HASH_TABLE_H_INCLUDED
#define HASH_TABLE_H_INCLUDED

#include "../../List/List.h"
#include "ErrorParser.h"
#include "HashFunctions.h"

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
size_t HashTableSearchAsmSearchSIMDHash(HashTable_t* HashTable, void* Key);
size_t HashTableSearchSIMDHashAsmStrcmp(HashTable_t* HashTable, void* Key);

size_t HashTableSearchAsmSearch(HashTable_t* HashTable, void* Key);
size_t HashTableSearch(HashTable_t* HashTable, void* Key);
size_t HashTableSearchSIMDHash(HashTable_t* HashTable, void* Key);
//HashErrors HashTableErase(char* key, HashTable_t* HashTable);

enum HashErrors HashTableDump(HashTable_t* HashTable);

HashErrors HashTableInsertNaive(char* key, HashTable_t* HashTable);
HashErrors HashTableInsertSIMDHash(char* key, HashTable_t* HashTable);
HashErrors HashTableInsertSIMDHashAsmStrcmp(char* key, HashTable_t* HashTable);
HashErrors HashTableInsertSIMDHashAsmSearch(char* key, HashTable_t* HashTable);

HashErrors HashTableDelete(const char* key, HashTable_t* HashTable);

char* FileToHashTableWordArray(FILE* fp, size_t* WordCounter);

extern "C" size_t ListSearch(const char* Key, void* listData, size_t ListSize);
//extern "C" int mm_strcmp32(void* Key, void* KeyFromBucket);


#endif