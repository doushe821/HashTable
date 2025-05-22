#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include "../Headers/HashTable.h"
#include "../List/List.h"
#include "../List/ListStruct.h"
#include "../Headers/ErrorParser.h"
#include "../Headers/FileBufferizer.h"
#include <emmintrin.h>
#include <xmmintrin.h>
#include <x86intrin.h>

#include "../Headers/HashFunctions.h"

const size_t WordLengthMax = 32;
const size_t listInitSize = 8192;

inline static int mm_strcmp32(void* Key, void* KeyFromBucket);
static int KeysComp(void* Key, void* KeyFromList);

HashTable_t HashTableInit(size_t BucketCount)
{
    HashTable_t HashTable = {};

    if(BucketCount == 0)
    {
        HashTable.ErrorCode = ZERO_BUCKET_COUNT;
        return HashTable;
    }

    List_t** KeysBucketArray = (List_t**)calloc(sizeof(List_t*), BucketCount);
    if(KeysBucketArray == NULL)
    {
        HashTable.ErrorCode = ALLOCATION_FAILURE;
        return HashTable;
    }

    List_t** ValuesBucketArray = (List_t**)calloc(sizeof(List_t*), BucketCount);  
    if(ValuesBucketArray == NULL)
    {
        free(KeysBucketArray);
        HashTable.ErrorCode = ALLOCATION_FAILURE;
        return HashTable;
    }

    HashTable.KeysBucketArray = KeysBucketArray;
    HashTable.ValuesBucketArray = ValuesBucketArray;
    HashTable.BucketCount = BucketCount;
    HashTable.ErrorCode = MODULE_SUCCESS;

    return HashTable;
}

char* FileToHashTableWordArray(FILE* fp, size_t* WordCounter)
{
    if(fp == NULL)
    {
        return NULL;
    }

    size_t DataSize = GetFileSize(fp);
    char* FileBuffer = FileToString(fp);

    size_t LocalWordCounter = 0;
    
    for(size_t i = 0; i < DataSize; i++)
    {
        char* nextptr = strchr(FileBuffer + i, '\n');
        LocalWordCounter++;
        i += (size_t)nextptr - (size_t)(FileBuffer + i);
    }

    char* DataBuffer = (char*)aligned_alloc(sizeof(__m256), LocalWordCounter * sizeof(__m256)); 

    size_t FileBufferIndex = 0;
    for(size_t i = 0; i < LocalWordCounter; i++)
    {
        char* nextptr = strchr(FileBuffer + FileBufferIndex, '\n');

        strncpy(DataBuffer + i * sizeof(__m256), FileBuffer + FileBufferIndex, (size_t)nextptr - (size_t)(FileBuffer + FileBufferIndex));
        FileBufferIndex += (size_t)nextptr - (size_t)(FileBuffer + FileBufferIndex) + 1;
    }
    free(FileBuffer);
    (*WordCounter) = LocalWordCounter;
    return DataBuffer;
}

HashErrors HashTableInsertNaive(char* key, HashTable_t* HashTable)
{
    if(strlen(key) > WordLengthMax)
    {
        HashTable->ErrorCode = KEY_IS_TOO_BIG;
        return KEY_IS_TOO_BIG;
    }
    if(HashTable == NULL)
    {
        HashTable->ErrorCode = NULL_HASH_TABLE_POINTER;
        return NULL_HASH_TABLE_POINTER;
    } 
    if(((size_t)key % WordLengthMax) != 0)
    {
        HashTable->ErrorCode = MISSALIGNMENT;
        return MISSALIGNMENT;
    }

    size_t hash = crc32Hash(key, WordLengthMax, HashTable->BucketCount);

    if(HashTable->KeysBucketArray[hash] == NULL)
    {

        ListInit(&HashTable->KeysBucketArray[hash], listInitSize, WordLengthMax, WordLengthMax);
        ListInit(&HashTable->ValuesBucketArray[hash], listInitSize, sizeof(size_t), 0);
        PushFront(HashTable->KeysBucketArray[hash], (void*)key);
        size_t count = 1;
        PushFront(HashTable->ValuesBucketArray[hash], &count);
    }
    size_t index = 0;
    for(size_t i = 0; i < GetLinearListSize((HashTable->KeysBucketArray[hash])) / WordLengthMax; i++)
    {
        if(strncmp((char*)key, (char*)HashTable->KeysBucketArray[hash]->data + i * WordLengthMax, strlen(key)) == 0)
        {
            index = i;
            break;
        }
    }
    if(index == 0)
    {
        size_t NewValue = 1;
        PushFront(HashTable->KeysBucketArray[hash], (void*)key);
        PushFront(HashTable->ValuesBucketArray[hash], &NewValue);
    }
    else
    {
        size_t value = 0;
        memcpy(&value, ListGetNodeValueInd(HashTable->ValuesBucketArray[hash], index), sizeof(value));
        value++;
        ListUpdateNodeValue(HashTable->ValuesBucketArray[hash], &value, index);
    }  
    return MODULE_SUCCESS;
}

HashErrors HashTableInsertSIMDHash(char* key, HashTable_t* HashTable)
{
    if(strlen(key) > WordLengthMax)
    {
        HashTable->ErrorCode = KEY_IS_TOO_BIG;
        return KEY_IS_TOO_BIG;
    }
    if(HashTable == NULL)
    {
        HashTable->ErrorCode = NULL_HASH_TABLE_POINTER;
        return NULL_HASH_TABLE_POINTER;
    } 
    if(((size_t)key % WordLengthMax) != 0)
    {
        HashTable->ErrorCode = MISSALIGNMENT;
        return MISSALIGNMENT;
    }

    size_t hash = crc32HashIntrinsics(key, HashTable->BucketCount); // TODO take module division here

    if(HashTable->KeysBucketArray[hash] == NULL)
    {

        ListInit(&HashTable->KeysBucketArray[hash], listInitSize, WordLengthMax, WordLengthMax);
        ListInit(&HashTable->ValuesBucketArray[hash], listInitSize, sizeof(size_t), 0);
        PushFront(HashTable->KeysBucketArray[hash], (void*)key);
        size_t count = 1;
        PushFront(HashTable->ValuesBucketArray[hash], &count);
    }
    size_t index = 0;
    for(size_t i = 0; i < GetLinearListSize((HashTable->KeysBucketArray[hash])) / WordLengthMax; i++)
    {
        if(strncmp((char*)key, (char*)HashTable->KeysBucketArray[hash]->data + i * WordLengthMax, strlen(key)) == 0)
        {
            index = i;
            break;
        }
    }
    if(index == 0)
    {
        size_t NewValue = 1;
        PushFront(HashTable->KeysBucketArray[hash], (void*)key);
        PushFront(HashTable->ValuesBucketArray[hash], &NewValue);
    }
    else
    {
        size_t value = 0;
        memcpy(&value, ListGetNodeValueInd(HashTable->ValuesBucketArray[hash], index), sizeof(value));
        value++;
        ListUpdateNodeValue(HashTable->ValuesBucketArray[hash], &value, index);
    }   
    return MODULE_SUCCESS;
}


HashErrors HashTableInsertSIMDHashAsmStrcmp(char* key, HashTable_t* HashTable)
{
    if(strlen(key) > WordLengthMax)
    {
        HashTable->ErrorCode = KEY_IS_TOO_BIG;
        return KEY_IS_TOO_BIG;
    }
    if(HashTable == NULL)
    {
        HashTable->ErrorCode = NULL_HASH_TABLE_POINTER;
        return NULL_HASH_TABLE_POINTER;
    } 
    if(((size_t)key % WordLengthMax) != 0)
    {
        HashTable->ErrorCode = MISSALIGNMENT;
        return MISSALIGNMENT;
    }

    size_t hash = crc32HashIntrinsics(key, HashTable->BucketCount);

    if(HashTable->KeysBucketArray[hash] == NULL)
    {

        ListInit(&HashTable->KeysBucketArray[hash], listInitSize, WordLengthMax, WordLengthMax);
        ListInit(&HashTable->ValuesBucketArray[hash], listInitSize, sizeof(size_t), 0);
        PushFront(HashTable->KeysBucketArray[hash], (void*)key);
        size_t count = 1;
        PushFront(HashTable->ValuesBucketArray[hash], &count);
    }

    size_t index = 0;
    for(size_t i = 0; i < GetLinearListSize((HashTable->KeysBucketArray[hash])) / WordLengthMax; i++)
    {
        if(mm_strcmp32((char*)key, (char*)HashTable->KeysBucketArray[hash]->data + i * WordLengthMax) == 0)
        {
            index = i;
            break;
        }
    }
    if(index == 0)
    {
        size_t NewValue = 1;
        PushFront(HashTable->KeysBucketArray[hash], (void*)key);
        PushFront(HashTable->ValuesBucketArray[hash], &NewValue);
    }
    else
    {
        size_t value = 0;
        memcpy(&value, ListGetNodeValueInd(HashTable->ValuesBucketArray[hash], index), sizeof(value));
        value++;
        ListUpdateNodeValue(HashTable->ValuesBucketArray[hash], &value, index);
    }        
    return MODULE_SUCCESS;
}


HashErrors HashTableInsertSIMDHashAsmSearch(char* key, HashTable_t* HashTable)
{
    if(strlen(key) > WordLengthMax)
    {
        HashTable->ErrorCode = KEY_IS_TOO_BIG;
        return KEY_IS_TOO_BIG;
    }
    if(HashTable == NULL)
    {
        HashTable->ErrorCode = NULL_HASH_TABLE_POINTER;
        return NULL_HASH_TABLE_POINTER;
    } 
    if(((size_t)key % WordLengthMax) != 0)
    {
        HashTable->ErrorCode = MISSALIGNMENT;
        return MISSALIGNMENT;
    }

    size_t hash = crc32HashIntrinsics(key, HashTable->BucketCount);

    if(HashTable->KeysBucketArray[hash] == NULL)
    {

        ListInit(&HashTable->KeysBucketArray[hash], listInitSize, WordLengthMax, WordLengthMax);
        ListInit(&HashTable->ValuesBucketArray[hash], listInitSize, sizeof(size_t), 0);
        PushFront(HashTable->KeysBucketArray[hash], (void*)key);
        size_t count = 1;
        PushFront(HashTable->ValuesBucketArray[hash], &count);
    }

    size_t index = 0;
    for(size_t i = 0; i < GetLinearListSize((HashTable->KeysBucketArray[hash])) / WordLengthMax; i++)
    {
        if(mm_strcmp32((char*)key, (char*)HashTable->KeysBucketArray[hash]->data + i * WordLengthMax) == 0)
        {
            index = i;
            break;
        }
    }
    if(index == 0)
    {
        size_t NewValue = 1;
        PushFront(HashTable->KeysBucketArray[hash], (void*)key);
        PushFront(HashTable->ValuesBucketArray[hash], &NewValue);
    }
    else
    {
        size_t value = 0;
        memcpy(&value, ListGetNodeValueInd(HashTable->ValuesBucketArray[hash], index), sizeof(value));
        value++;
        ListUpdateNodeValue(HashTable->ValuesBucketArray[hash], &value, index);
    }        
    return MODULE_SUCCESS;
}

size_t HashTableSearchNaive(HashTable_t* HashTable, void* Key)
{
    __asm__ __volatile__
    (
        "NaiveSearch:"
    );

    if(HashTable == NULL)
    {
        fprintf(stderr, "Hash table pointer is NULL\n");
        return NULL_HASH_TABLE_POINTER;
    }
    if(Key == NULL)
    {
        fprintf(stderr, "Key pointer is NULL");
        return NULL_KEY_POINTER;
    }

    size_t hash = crc32Hash(Key, WordLengthMax, HashTable->BucketCount);
    if(HashTable->KeysBucketArray[hash] == NULL)
    {
        return 0;
    }

    size_t index = ListSearchInd(HashTable->KeysBucketArray[hash], Key, KeysComp);

    if(index == 0)
    {
        return 0;
    }

    size_t Number =  *(size_t*)((char*)HashTable->ValuesBucketArray[hash]->data + index * HashTable->ValuesBucketArray[hash]->elsize);

    return Number;
}


size_t HashTableSearchSIMDHash(HashTable_t* HashTable, void* Key)
{
    __asm__ __volatile__
    (
        "FirstOptSearch:"
    );
    if(HashTable == NULL)
    {
        fprintf(stderr, "Hash table pointer is NULL\n");
        return NULL_HASH_TABLE_POINTER;
    }
    if(Key == NULL)
    {
        fprintf(stderr, "Key pointer is NULL");
        return NULL_KEY_POINTER;
    }

    size_t hash = crc32HashIntrinsics(Key, HashTable->BucketCount);

    if(HashTable->KeysBucketArray[hash] == NULL)
    {
        return 0;
    }

    size_t index = ListSearchInd(HashTable->KeysBucketArray[hash], Key, KeysComp);

    if(index == 0)
    {
        return 0;
    }

    size_t Number =  *(size_t*)((char*)HashTable->ValuesBucketArray[hash]->data + index * HashTable->ValuesBucketArray[hash]->elsize);

    return Number;
}


static int KeysComp(void* Key, void* KeyFromList)
{
    char* KeyStr = (char*)Key;
    char* KeyStrFromList = (char*)KeyFromList;
    return strncmp(KeyStr, KeyStrFromList, WordLengthMax);
}

enum HashErrors HashTableDump(HashTable_t* HashTable)
{
    FILE* dumpFile = fopen("HashDump.txt", "w+b");
    if(dumpFile == NULL)
    {
        return FILE_NULL_POINTER;
    }
    NodeInfo infoKeys = {};
    size_t Number = 0;
    char string[WordLengthMax] = {};
    for(size_t i = 0; i < HashTable->BucketCount; i++)
    {
        if(HashTable->KeysBucketArray[i] != NULL)
        {
            size_t index = 0;
            infoKeys = LNodeInfo(HashTable->KeysBucketArray[i], index);
            index = infoKeys.next;
            do
            {
                void* Key = (char*)HashTable->KeysBucketArray[i]->data + index * HashTable->KeysBucketArray[i]->elsize;//ListGetNodeValueInd(HashTable->BucketArray[i], index);
    
                memcpy(string, Key, WordLengthMax);
    
                fprintf(dumpFile, "%s ", string);
    
                memcpy(&Number, (char*)HashTable->ValuesBucketArray[i]->data + HashTable->ValuesBucketArray[i]->elsize * index, sizeof(Number));
                
                fprintf(dumpFile, "%zu\n", Number);

                infoKeys = LNodeInfo(HashTable->KeysBucketArray[i], index);
                index = infoKeys.next;
            }while(index != 0);
        }
    }
    fclose(dumpFile);
    return MODULE_SUCCESS;
}

int HashTableDestr(HashTable_t* HashTable)
{
    for(size_t i = 0; i < HashTable->BucketCount; i++)
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

size_t HashTableSearchAsmSearch(HashTable_t* HashTable, void* Key) 
{
    if(HashTable == NULL)
    {
        return NULL_HASH_TABLE_POINTER;
    }
    if(Key == NULL)
    {
        return NULL_KEY_POINTER;
    }

    size_t hash = crc32HashIntrinsics(Key, HashTable->BucketCount);

    if(HashTable->KeysBucketArray[hash] == NULL)
    {

        return 0;     
    }

    size_t index = 0;
    size_t ListSize = GetLinearListSize(HashTable->KeysBucketArray[hash]) / HashTable->KeysBucketArray[hash]->elsize;
    uint8_t result = 0;
    for(size_t i = 1; i < ListSize; i++)
    {

        __asm__ __volatile__
        (
            "vmovaps (%1), %%ymm0\n"           
            "vmovaps (%2), %%ymm1\n\t"         
            "vpxor %%ymm1, %%ymm0, %%ymm0\n\t" 
            "vptest %%ymm0, %%ymm0\n\t"        
            "setne %0\n\t"                   
            : "=r" (result)                    
            : "D" (Key), "S" ((char*)HashTable->KeysBucketArray[hash]->data + i * HashTable->KeysBucketArray[hash]->elsize)   
            : "ymm0", "ymm1", "cc", "memory"
        );

        if(result == 0)
        {
            index = i;
            break;
        }
    }

    size_t Number =  *(size_t*)((char*)HashTable->ValuesBucketArray[hash]->data + index * HashTable->ValuesBucketArray[hash]->elsize);

    return Number;
}


size_t HashTableSearchSIMDHashAsmStrcmp(HashTable_t* HashTable, void* Key) 
{
    if(HashTable == NULL)
    {
        return NULL_HASH_TABLE_POINTER;
    }
    if(Key == NULL)
    {
        return NULL_KEY_POINTER;
    }

    size_t hash = crc32HashIntrinsics(Key, HashTable->BucketCount);

    if(HashTable->KeysBucketArray[hash] == NULL)
    {

        return 0;     
    }

    size_t index = ListSearchInd(HashTable->KeysBucketArray[hash], Key, mm_strcmp32);

    if(index == 0)
    {
        return 0;
    }

    size_t Number =  *(size_t*)((char*)HashTable->ValuesBucketArray[hash]->data + index * HashTable->ValuesBucketArray[hash]->elsize);

    return Number;
}

inline static int mm_strcmp32(void* key, void* KeyFromList)
{
    uint8_t result = 1;
    __asm__ __volatile__
    (
        "vmovaps (%1), %%ymm0\n"           
        "vmovaps (%2), %%ymm1\n\t"         
        "vpxor %%ymm1, %%ymm0, %%ymm0\n\t" 
        "vptest %%ymm0, %%ymm0\n\t"        
        "setne %0\n\t"                   
        : "=r" (result)                    
        : "D" (key), "S" (KeyFromList)     
        : "ymm0", "ymm1", "cc", "memory"
    );
    
    return (int)result;
}