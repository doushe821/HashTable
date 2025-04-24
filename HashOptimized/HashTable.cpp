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

const size_t WordLengthMax = 32;
const size_t listInitSize = 8192;

static size_t WhackyHash(void* Key, size_t len, size_t MaxValue);

static size_t WhackyHash2(void* Key, size_t len, size_t MaxValue);

static size_t MurmurHash2 (const char * key, size_t len, size_t MaxValue);
static size_t WhackyHash2_SIMD(void* Key, size_t len, size_t MaxValue);

static int KeysComp(void* Key, void* KeyFromList);

static enum HashErrors ListInsertOrIncrementHashTable(char* key, HashTable_t* HashTable, size_t bucketIndex);

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
    memset(DataBuffer, 0, LocalWordCounter * sizeof(__m256));
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

static size_t WhackyHash(void* Key, size_t len, size_t MaxValue)
{
    size_t semen = 0x69696ABE1;
    size_t MLG = 0xABE1;
    size_t tony = 0xEDA666;
    
    size_t HashSum = 0;
    for(size_t i = 0; i < len / 4; i++)
    {
        int HashSubSum = ((char*)Key)[i * 4] +  ((char*)Key)[i * 4 + 1] + ((char*)Key)[i * 4 + 2] + ((char*)Key)[i * 4 + 3];
        HashSum += ((HashSubSum ^ semen) * MLG);
        HashSum ^= semen;
        HashSum *= MLG;
    }

    return HashSum % MaxValue;
}

static size_t WhackyHash2(void* Key, size_t len, size_t MaxValue)
{
    size_t seed = 0x69696ABE1;
    size_t MLG = 0xABE1;
    size_t tony = 0xEDA666EDA6667878;
    
    size_t HashSum = 0;
    for(size_t i = 0; i < len / 4; i++)
    {
        int HashSubSum = ((char*)Key)[i * 4] +  ((char*)Key)[i * 4 + 1] + ((char*)Key)[i * 4 + 2] + ((char*)Key)[i * 4 + 3];
        HashSum += ((HashSubSum ^ seed) * MLG);
        HashSum ^= seed;
        HashSum *= MLG;
        HashSum ^= tony;
        HashSum ^= MaxValue;
    }

    return HashSum % MaxValue;
}

static size_t WhackyHash2_SIMD(void* Key, size_t len, size_t MaxValue)
{
    size_t seed = 0x69696ABE1;
    size_t MLG = 0xABE1;
    size_t tony = 0xEDA666EDA6667878;
    
    __m256i seedm256 = _mm256_set1_epi64x(seed);

    size_t HashSubSums[4] = {};
    for(size_t i = 0; i < 4; i++)
    {
        HashSubSums[i] = ((char*)Key)[i * 8] +  ((char*)Key)[i * 8 + 1] + ((char*)Key)[i * 8 + 2] + ((char*)Key)[i * 8 + 3]
        + ((char*)Key)[i * 8 + 4] +  ((char*)Key)[i * 8 + 5] + ((char*)Key)[i * 8 + 6] + ((char*)Key)[i * 8 + 7];
    }

    __m256i SubSums256 = {(long long)HashSubSums[0], (long long)HashSubSums[1], (long long)HashSubSums[2], (long long)HashSubSums[3]};
    SubSums256 = _mm256_xor_si256(SubSums256, seedm256);

    size_t HashSum = SubSums256[0] + SubSums256[1] + SubSums256[2] + SubSums256[3];
    return HashSum % MaxValue;
}

static size_t MurmurHash2 (const char * key, size_t len, size_t MaxValue)
{
  size_t m = 0x5bd1e995;
  size_t seed = 0;
  size_t r = 24;

  size_t h = seed ^ len;

  const unsigned char * data = (const unsigned char *)key;
  unsigned int k = 0;

  while (len >= 4)
  {
      k  = data[0];
      k |= data[1] << 8;
      k |= data[2] << 16;
      k |= data[3] << 24;

      k *= m;
      k ^= k >> r;
      k *= m;

      h *= m;
      h ^= k;

      data += 4;
      len -= 4;
  }

  switch (len)
  {
    case 3:
      h ^= data[2] << 16;
    case 2:
      h ^= data[1] << 8;
    case 1:
      h ^= data[0];
      h *= m;
  };

  h ^= h >> 13;
  h *= m;
  h ^= h >> 15;

  return h % MaxValue;
}

HashErrors HashTableInsert(const char* key, HashTable_t* HashTable)
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

    //size_t hash = WhackyHash((void*)key, WordLengthMax, HashTable->BucketCount);
    //size_t hash = MurmurHash2(key, WordLengthMax, HashTable->BucketCount);
    //size_t hash = WhackyHash2((void*)key, WordLengthMax, HashTable->BucketCount);
    size_t hash = WhackyHash2_SIMD((void*)key, WordLengthMax, HashTable->BucketCount);

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
        size_t index = ListSearchInd(HashTable->KeysBucketArray[hash], (void*)key, KeysComp);
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
    }
    return MODULE_SUCCESS;
}


size_t HashTableSearchNaive(HashTable_t* HashTable, void* Key)
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

    size_t hash = WhackyHash2(Key, WordLengthMax, HashTable->BucketCount);
    if(HashTable->KeysBucketArray[hash] == NULL)
    {
        //fprintf(stderr, "Bucket isn't loaded\n");
        return 0;
    }

    size_t index = ListSearchInd(HashTable->KeysBucketArray[hash], Key, KeysComp);

    size_t Number =  *(size_t*)((char*)HashTable->ValuesBucketArray[hash]->data + index * HashTable->ValuesBucketArray[hash]->elsize);

    return Number;
}

size_t HashTableSearchHashOptimized(HashTable_t* HashTable, void* Key)
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

    size_t hash = WhackyHash2_SIMD(Key, WordLengthMax, HashTable->BucketCount);
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

HashErrors HashTableErase(const char* key, HashTable_t* HashTable) // TODO VOID or char
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

    size_t hash = 0;
    asm volatile
    ( 
        "xor %%rax, %%rax\t\n"
        "xor %%rdx, %%rdx\t\n"

        "movq %1, %%rsi\t\n"
        "movq $32, %%rcx\t\n"

        ".HashEraseLoop:\t\n"

        "movb (%%rsi), %%dl\t\n"
        "addq %%rdx, %%rax\t\n"

        "add $1, %%rsi\t\n"

        "dec %%rcx\t\n"
        "cmp $0, %%rcx\t\n"
        "jne .HashEraseLoop\t\n"

        "movq %%rax, %0\t\n"
        :"=r" (hash)
        :"r" (key)
        :"rax", "rbx", "rcx", "rdx", "rsi", "memory"
    );

    size_t index = ListSearch(key, HashTable->KeysBucketArray[hash]->data, GetLinearListSize((HashTable->KeysBucketArray[hash])));
    if(index == 0)
    {
        HashTable->ErrorCode = NO_KEY_TO_DELETE;
        return NO_KEY_TO_DELETE;
    }
    else
    {   
        memset((char*)HashTable->KeysBucketArray[hash]->data + index * WordLengthMax, 0, WordLengthMax);
        memset((char*)HashTable->ValuesBucketArray[hash]->data + index * sizeof(index), 0, sizeof(index));
    }
    return MODULE_SUCCESS;
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

static enum HashErrors ListInsertOrIncrementHashTable(char* key, HashTable_t* HashTable, size_t bucketIndex)
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

size_t HashTableSearch(HashTable_t* HashTable, void* Key) // TODO rename error parser structure
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