#ifndef HASH_TABLE_H_INCLUDED
#define HASH_TABLE_H_INCLUDED

#include "List/List.h"
#include "ErrorParser.h"

struct HTError
{
    ErrorCodes Code;
    char FileName[FILENAME_MAX];
    int Line;

};

struct HashTable_t 
{
    List_t** KeysBucketArray;
    List_t** ValuesBucketArray;
    HTError ErrorInfo;
    size_t HashFunction(void* key, size_t KeySize);
};

#define _FILL_ERROR_INFO(code, file, line) HashTable.ErrorInfo.Code = code; HashTable.ErrorInfo.Line = line; strncpy(HashTable.ErrorInfo.FileName, file, strlen(file));
#define _FILL_ERROR_INFO_PTR(code, file, line) HashTable->ErrorInfo.Code = code; HashTable->ErrorInfo.Line = line; strncpy(HashTable->ErrorInfo.FileName, file, strlen(file));

HashTable_t HashTableInit(FILE* fp);

int HashTableDestr(HashTable_t* HashTable);

enum ErrorCodes HashTableSearch(HashTable_t* HashTable, void* Key);

enum ErrorCodes HashTableDump(HashTable_t* HashTable);

ErrorCodes HashTableInsert(const char* key, HashTable_t* HashTable);
ErrorCodes HashTableDelete(const char* key, HashTable_t* HashTable);

size_t SimpleHash(void* value, size_t size);

extern "C" size_t ListSearch(const char* Key, void* listData, size_t ListSize);

#endif