#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include "HashTable.h"
#include "List/List.h"
#include "List/ListStruct.h"
#include "ErrorParser.h"
#include "FileBufferizer.h"
#include <emmintrin.h>
#include <xmmintrin.h>
#include <x86intrin.h>

#include "HashFunctions.h"

const size_t WordLengthMax = 32;
const size_t listInitSize = 8192;

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

HashErrors HashTableInsertNaive(char* key, HashTable_t* HashTable, uint32_t* crc32Table)
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

    size_t hash = crc32Hash(key, WordLengthMax, HashTable->BucketCount, crc32Table);

    if(HashTable->KeysBucketArray[hash] == NULL)
    {

        ListInit(&HashTable->KeysBucketArray[hash], listInitSize, WordLengthMax, WordLengthMax);
        ListInit(&HashTable->ValuesBucketArray[hash], listInitSize, sizeof(size_t), 0);
        PushFront(HashTable->KeysBucketArray[hash], (void*)key);
        size_t count = 1;
        PushFront(HashTable->ValuesBucketArray[hash], &count);
    }
    else
    {
        size_t index = 0;
        for(size_t i = 0; i < GetLinearListSize((HashTable->KeysBucketArray[hash])) / WordLengthMax; i++)
        {
            if(strncmp((char*)key, (char*)HashTable->KeysBucketArray[hash]->data + i * WordLengthMax, WordLengthMax) == 0)
            {
                index = i;
                break;
            }
        }
        if(index == 0)
        {
            size_t NewValue = 1;
            PushInd(HashTable->KeysBucketArray[hash], (void*)key, GetLinearListSize((HashTable->KeysBucketArray[hash])) + 1);
            PushInd(HashTable->ValuesBucketArray[hash], &NewValue, GetLinearListSize((HashTable->ValuesBucketArray[hash])) + 1);
        }
        else
        {
            size_t value = 0;
            memcpy(&value, ListGetNodeValueInd(HashTable->ValuesBucketArray[hash], index), sizeof(value));
            value++;
            ListUpdateNodeValue(HashTable->ValuesBucketArray[hash], &value, index);
        }        
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

    size_t hash = crc32HashIntrinsics(key, HashTable->BucketCount);

    if(HashTable->KeysBucketArray[hash] == NULL)
    {

        ListInit(&HashTable->KeysBucketArray[hash], listInitSize, WordLengthMax, WordLengthMax);
        ListInit(&HashTable->ValuesBucketArray[hash], listInitSize, sizeof(size_t), 0);
        PushFront(HashTable->KeysBucketArray[hash], (void*)key);
        size_t count = 1;
        PushFront(HashTable->ValuesBucketArray[hash], &count);
    }
    else
    {
        size_t index = 0;
        for(size_t i = 0; i < GetLinearListSize((HashTable->KeysBucketArray[hash])) / WordLengthMax; i++)
        {
            if(strncmp((char*)key, (char*)HashTable->KeysBucketArray[hash]->data + i * WordLengthMax, WordLengthMax) == 0)
            {
                index = i;
                break;
            }
        }
        if(index == 0)
        {
            size_t NewValue = 1;
            PushInd(HashTable->KeysBucketArray[hash], (void*)key, GetLinearListSize((HashTable->KeysBucketArray[hash])) + 1);
            PushInd(HashTable->ValuesBucketArray[hash], &NewValue, GetLinearListSize((HashTable->ValuesBucketArray[hash])) + 1);
        }
        else
        {
            size_t value = 0;
            memcpy(&value, ListGetNodeValueInd(HashTable->ValuesBucketArray[hash], index), sizeof(value));
            value++;
            ListUpdateNodeValue(HashTable->ValuesBucketArray[hash], &value, index);
        }        
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
    else
    {
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
            PushInd(HashTable->KeysBucketArray[hash], (void*)key, GetLinearListSize((HashTable->KeysBucketArray[hash])) + 1);
            PushInd(HashTable->ValuesBucketArray[hash], &NewValue, GetLinearListSize((HashTable->ValuesBucketArray[hash])) + 1);
        }
        else
        {
            size_t value = 0;
            memcpy(&value, ListGetNodeValueInd(HashTable->ValuesBucketArray[hash], index), sizeof(value));
            value++;
            ListUpdateNodeValue(HashTable->ValuesBucketArray[hash], &value, index);
        }        
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

    size_t hash = crc32HashIntrinsics((void*)key, WordLengthMax);

    if(HashTable->KeysBucketArray[hash] == NULL)
    {

        ListInit(&HashTable->KeysBucketArray[hash], listInitSize, WordLengthMax, WordLengthMax);
        ListInit(&HashTable->ValuesBucketArray[hash], listInitSize, sizeof(size_t), 0);
        PushFront(HashTable->KeysBucketArray[hash], (void*)key);
        size_t count = 1;
        PushFront(HashTable->ValuesBucketArray[hash], &count);
    }
    else
    {
        size_t index = ListSearch(key, HashTable->KeysBucketArray[hash]->data, GetLinearListSize((HashTable->KeysBucketArray[hash])));
        if(index == 0)
        {
            size_t NewValue = 1;
            PushFront(HashTable->KeysBucketArray[hash], (void*)key);//, GetLinearListSize((HashTable->KeysBucketArray[hash])) + 1);
            PushFront(HashTable->ValuesBucketArray[hash], &NewValue);//, //GetLinearListSize((HashTable->ValuesBucketArray[hash])) + 1);
        }
        else
        {
            size_t value = 0;
            memcpy(&value, ListGetNodeValueInd(HashTable->ValuesBucketArray[hash], index), sizeof(value));
            value++;
            ListUpdateNodeValue(HashTable->ValuesBucketArray[hash], &value, index);
        }        
    }
    return MODULE_SUCCESS;
}

size_t HashTableSearchNaive(HashTable_t* HashTable, void* Key, uint32_t* crc32table)
{
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

    size_t hash = crc32Hash(Key, WordLengthMax, HashTable->BucketCount, crc32table);
    if(HashTable->KeysBucketArray[hash] == NULL)
    {
        //fprintf(stderr, "Bucket isn't loaded\n");
        return 0;
    }

    size_t index = ListSearchInd(HashTable->KeysBucketArray[hash], Key, KeysComp);

    size_t Number =  *(size_t*)((char*)HashTable->ValuesBucketArray[hash]->data + index * HashTable->ValuesBucketArray[hash]->elsize);

    return Number;
}

static int KeysComp(void* Key, void* KeyFromList)
{
    char* KeyStr = (char*)Key;
    char* KeyStrFromList = (char*)KeyFromList;

    return strncmp(KeyStr, KeyStrFromList, WordLengthMax);
}

//size_t HashTableSearchSIMDHashAsmStrcmp(HashTable_t* HashTable, void* Key)
//{
//    if(HashTable == NULL)
//    {
//        fprintf(stderr, "Hash table pointer is NULL\n");
//        return NULL_HASH_TABLE_POINTER;
//    }
//    if(Key == NULL)
//    {
//        fprintf(stderr, "Key pointer is NULL");
//        return NULL_KEY_POINTER;
//    }
//
//    size_t hash = crc32HashIntrinsics(Key, HashTable->BucketCount);
//    if(HashTable->KeysBucketArray[hash] == NULL)
//    {
//        //fprintf(stderr, "Bucket isn't loaded\n");
//        return 0;
//    }
//
//    size_t index = 0;
//
//    //size_t index = ListSearchInd(HashTable->KeysBucketArray[hash], Key, mm_strcmp32);
//
//    size_t ElemIndex = HashTable->KeysBucketArray[hash]->ref[0].next;
//    while(ElemIndex != 0)
//    {
//        if(comp((char*)list->data + ElemIndex * list->elsize, Key) == 0)
//        {
//            return ;
//        }
//    
//        ElemIndex = list->ref[ElemIndex].next;
//
//    }
//
//    size_t Number =  *(size_t*)((char*)HashTable->ValuesBucketArray[hash]->data + index * HashTable->ValuesBucketArray[hash]->elsize);
//
//    return Number;
//}

//HashErrors HashTableErase(char* key, HashTable_t* HashTable) 
//{
//    if(strlen(key) > WordLengthMax)
//    {
//        HashTable->ErrorCode = KEY_IS_TOO_BIG;
//        return KEY_IS_TOO_BIG;
//    }
//    if(HashTable == NULL)
//    {
//        HashTable->ErrorCode = NULL_HASH_TABLE_POINTER;
//        return NULL_HASH_TABLE_POINTER;
//    }
//
//    size_t hash = WhackyHash2_SIMD((void*)key, sizeof(__m256), HashTable->BucketCount);
//
//    size_t index = ListSearch(key, HashTable->KeysBucketArray[hash]->data, GetLinearListSize((HashTable->KeysBucketArray[hash]))); 
//    if(index == 0)
//    {
//        HashTable->ErrorCode = NO_KEY_TO_DELETE;
//        return NO_KEY_TO_DELETE;
//    }
//    else
//    {   
//        memset((char*)HashTable->KeysBucketArray[hash]->data + index * WordLengthMax, 0, WordLengthMax);
//        memset((char*)HashTable->ValuesBucketArray[hash]->data + index * sizeof(index), 0, sizeof(index));
//    }
//    return MODULE_SUCCESS;
//}

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

    size_t index = ListSearch((char*)Key, HashTable->KeysBucketArray[hash]->data, GetLinearListSize(HashTable->KeysBucketArray[hash]));

    size_t Number =  *(size_t*)((char*)HashTable->ValuesBucketArray[hash]->data + index * HashTable->ValuesBucketArray[hash]->elsize);

    return Number;
}

size_t HashTableSearchSIMDHash(HashTable_t* HashTable, void* Key)
{
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
        //fprintf(stderr, "Bucket isn't loaded\n");
        return 0;
    }

    size_t index = ListSearchInd(HashTable->KeysBucketArray[hash], Key, KeysComp);

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

  //  size_t index = 0;
  //  size_t ListSize = GetLinearListSize(HashTable->KeysBucketArray[hash]);
  //  
  //  for(size_t i = 0; i < ListSize; i++)
  //  {
  //      void* ListElemValue = (char*)HashTable->KeysBucketArray[hash]->data + i * WordLengthMax;
  //      if(mm_strcmp32(Key, ListElemValue) == 0)
  //      {
  //          index = i;
  //          break;
  //      }
  //  }
    size_t index = ListSearchInd(HashTable->KeysBucketArray[hash], Key, mm_strcmp32);

    size_t Number =  *(size_t*)((char*)HashTable->ValuesBucketArray[hash]->data + index * HashTable->ValuesBucketArray[hash]->elsize);

    return Number;
}

int mm_strcmp32(void* key, void* KeyFromList)
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