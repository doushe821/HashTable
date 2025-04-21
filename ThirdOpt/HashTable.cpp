#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "HashTable.h"
#include "List/List.h"
#include "List/ListStruct.h"
#include "ErrorParser.h"
#include "FileBufferizer.h"
#include <emmintrin.h>
#include <xmmintrin.h>
#include <x86intrin.h>

const size_t HashTableWidth = 256; 
const size_t WordLengthMax = 32;
const size_t listInitSize = 4096;

static enum ErrorCodes ListInsertOrIncrementHashTable(char* key, HashTable_t* HashTable, size_t bucketIndex);

static int KeysComp(void* Key, void* KeyFromList);

HashTable_t HashTableInit(FILE* fp)
{
    char* buffer = FileToString(fp);
    size_t bufferSize = GetFileSize(fp);
    HashTable_t HashTable = {};

    List_t** KeysBucketArray = (List_t**)aligned_alloc(sizeof(__m256), sizeof(List_t*) * HashTableWidth);
    if(KeysBucketArray == NULL)
    {
        HashTable.ErrorCode = ALLOCATION_FAILURE;
        free(buffer);
        return HashTable;
    }
    List_t** ValuesBucketArray = (List_t**)aligned_alloc(sizeof(__m256), sizeof(List_t*) * HashTableWidth);  

    if(ValuesBucketArray == NULL)
    {
        free(KeysBucketArray);
        free(buffer);
        HashTable.ErrorCode = ALLOCATION_FAILURE;
        return HashTable;
    }

    HashTable.KeysBucketArray = KeysBucketArray;
    HashTable.ValuesBucketArray = ValuesBucketArray;

    char* word = (char*)aligned_alloc(sizeof(__m256), WordLengthMax);

    for(size_t CharsRead = 0; CharsRead < bufferSize; CharsRead++)
    {
        char* wordEnd = strchr(buffer + CharsRead, '\n');
        size_t wordLength = (size_t)wordEnd - (size_t)(buffer + CharsRead);
        if(wordLength >= WordLengthMax)
        {
            CharsRead += wordLength;
            break;
        }
        strncpy(word, buffer + CharsRead, wordLength);
        word[wordLength] = '\0';

        CharsRead += wordLength;
        
        size_t hash = 0;

        asm volatile
        ( 
            "xor %%rax, %%rax\t\n"
            "xor %%rdx, %%rdx\t\n"
    
            "movq %1, %%rsi\t\n"
            "movq $32, %%rcx\t\n"
    
            ".HashInitLoop:\t\n"
    
            "movb (%%rsi), %%dl\t\n"
            "addq %%rdx, %%rax\t\n"
    
            "add $1, %%rsi\t\n"
    
            "dec %%rcx\t\n"
            "cmp $0, %%rcx\t\n"
            "jne .HashInitLoop\t\n"
    
            "movq %%rax, %0\t\n"
            :"=r" (hash)
            :"r" (word)
            :"rax", "rbx", "rcx", "rdx", "rsi", "memory"
        );
    
        hash %= 256;
        
        if(KeysBucketArray[hash] == NULL)
        {
            ListInit(&(KeysBucketArray[hash]), listInitSize, WordLengthMax, sizeof(__m256));
            ListInit(&(ValuesBucketArray[hash]), listInitSize, sizeof(size_t), 0);
        }

        ParseError(ListInsertOrIncrementHashTable(word, &HashTable, hash)); 

        asm volatile
        ( 
            "vpxor %%ymm0, %%ymm0, %%ymm0\t\n"
            "vmovaps %%ymm0, (%0)"
            :"=r" (word)
            :"r" (word)
            :"memory", "ymm0"
        );
    }
    free(word);
    free(buffer);
    return HashTable;
}

inline size_t SimpleHash(void* value, size_t size)
{
    const int TableWidth = 256;
    size_t HashSum = 0;
    for(size_t i = 0; i < size; i++)
    {
        HashSum += (*((char*)value + i));
    }
    return HashSum % TableWidth;
}

enum ErrorCodes HashTableDump(HashTable_t* HashTable)
{
    FILE* dumpFile = fopen("HashDump.txt", "w+b");
    if(dumpFile == NULL)
    {
        return FILE_NULL_POINTER;
    }
    NodeInfo infoKeys = {};
    size_t Number = 0;
    char string[WordLengthMax] = {};
    for(size_t i = 0; i < HashTableWidth; i++)
    {
        if(HashTable->KeysBucketArray[i] != NULL)
        {
            size_t index = 0;
            infoKeys = LNodeInfo(HashTable->KeysBucketArray[i], index);
            index = infoKeys.next;
            while(index != 0)
            {
                void* Key = (char*)HashTable->KeysBucketArray[i]->data + index * HashTable->KeysBucketArray[i]->elsize;//ListGetNodeValueInd(HashTable->BucketArray[i], index);
    
                memcpy(string, Key, WordLengthMax);
    
                fprintf(dumpFile, "%s ", string);
    
                memcpy(&Number, (char*)HashTable->ValuesBucketArray[i]->data + HashTable->ValuesBucketArray[i]->elsize * index, sizeof(Number));
                
                fprintf(dumpFile, "%zu\n", Number);

                infoKeys = LNodeInfo(HashTable->KeysBucketArray[i], index);
                index = infoKeys.next;
            }
        }
    }
    fclose(dumpFile);
    return MODULE_SUCCESS;
}

int HashTableDestr(HashTable_t* HashTable)
{
    for(size_t i = 0; i < HashTableWidth; i++)
    {

        void* address = (List_t*)(HashTable->KeysBucketArray[i]);
        if(address != NULL)
        {
            ListDstr((List_t*)address);
            address = (List_t*)(HashTable->ValuesBucketArray[i]);
            ListDstr((List_t*)address);
        }
    }

    free(HashTable->KeysBucketArray);
    free(HashTable->ValuesBucketArray);
    return MODULE_SUCCESS;
}

static enum ErrorCodes ListInsertOrIncrementHashTable(char* key, HashTable_t* HashTable, size_t bucketIndex)
{
    size_t listSize =  GetLinearListSize(HashTable->KeysBucketArray[bucketIndex]);
    size_t index = ListSearch(key, HashTable->KeysBucketArray[bucketIndex]->data, listSize);
    if(index == 0)
    {
        size_t counterInit = 1;
        PushFront(HashTable->KeysBucketArray[bucketIndex], key);
        PushFront(HashTable->ValuesBucketArray[bucketIndex], &counterInit);
    }
    else
    {
        (*((size_t*)((char*)HashTable->ValuesBucketArray[bucketIndex]->data + index * HashTable->ValuesBucketArray[bucketIndex]->elsize)))++;
    }

    return MODULE_SUCCESS;
}

enum ErrorCodes HashTableSearch(HashTable_t* HashTable, void* Key)
{
    if(HashTable == NULL)
    {
        return NULL_HASH_TABLE_POINTER;
    }
    if(Key == NULL)
    {
        return NULL_KEY_POINTER;
    }

    size_t hash = 0;

    asm volatile
    ( 
        "xor %%rax, %%rdx\t\n"
        "xor %%rdx, %%rdx\t\n"

        "movq %1, %%rsi\t\n"
        "movq $32, %%rcx\t\n"

        ".HashSearchLoop:\t\n"

        "movb (%%rsi), %%dl\t\n"
        "addq %%rdx, %%rax\t\n"

        "add $1, %%rsi\t\n"

        "dec %%rcx\t\n"
        "cmp $0, %%rcx\t\n"
        "jne .HashSearchLoop\t\n"

        "movq %%rax, %0\t\n"
        :"=r" (hash)
        :"r" (Key)
        :"rax", "rbx", "rcx", "rdx", "rsi", "memory"
    );

    hash %= 256;

    if(HashTable->KeysBucketArray[hash] == NULL)
    {
        fprintf(stdout, "No such word in the table\n");
        return MODULE_SUCCESS;     
    }
    size_t index = ListSearch((char*)Key, HashTable->KeysBucketArray[hash]->data, GetLinearListSize(HashTable->KeysBucketArray[hash]));
    if(index == 0)
    {
        fprintf(stdout, "No such word in the table\n");
        return MODULE_SUCCESS;
    }
    void* Value = (char*)HashTable->KeysBucketArray[hash]->data + index * HashTable->KeysBucketArray[hash]->elsize;

    char string[WordLengthMax] = {};
    memcpy(string, Value, WordLengthMax);
    
    fprintf(stdout, "%s ", string);

    size_t Number =  *(size_t*)((char*)HashTable->ValuesBucketArray[hash]->data + index * HashTable->ValuesBucketArray[hash]->elsize);

    fprintf(stdout, "%zu\n", Number);

    return MODULE_SUCCESS;
}

static int KeysComp(void* Key, void* KeyFromList)
{
    char* KeyStr = (char*)Key;
    char* KeyStrFromList = (char*)KeyFromList;
    return strcmp(KeyStr, KeyStrFromList);
}