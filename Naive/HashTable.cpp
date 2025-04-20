#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "HashTable.h"
#include "List/List.h"
#include "List/ListStruct.h"
#include "ErrorParser.h"
#include "hash.h"
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

    List_t** KeysBucketArray = (List_t**)calloc(sizeof(List_t*), HashTableWidth);
    List_t** ValuesBucketArray = (List_t**)calloc(sizeof(List_t*), HashTableWidth);

    if(KeysBucketArray == NULL)
    {
        if(ValuesBucketArray == NULL)
        {
            free(ValuesBucketArray);
        }
        free(buffer);
        HashTable.ErrorCode = ALLOCATION_FAILURE;
        return HashTable;
    }

    if(ValuesBucketArray == NULL)
    {
        free(KeysBucketArray);
        free(buffer);
        HashTable.ErrorCode = ALLOCATION_FAILURE;
        return HashTable;
    }

    HashTable.KeysBucketArray = KeysBucketArray;
    HashTable.ValuesBucketArray = ValuesBucketArray;

    char word[WordLengthMax] = {};
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

        size_t hash = SimpleHash(word, WordLengthMax);
        
        if(KeysBucketArray[hash] == NULL)
        {
            ListInit(&(KeysBucketArray[hash]), listInitSize, WordLengthMax);
            ListInit(&(ValuesBucketArray[hash]), listInitSize, sizeof(size_t));
        }

        ParseError(ListInsertOrIncrementHashTable(word, &HashTable, hash)); 
    }
    free(buffer);
    return HashTable;
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
    size_t index = ListSearchInd(HashTable->KeysBucketArray[bucketIndex], key, KeysComp); 
    if(index == 0)
    {
        size_t counterInit = 1;
        PushFront(HashTable->KeysBucketArray[bucketIndex], key);
        PushFront(HashTable->ValuesBucketArray[bucketIndex], &counterInit);

        //void* value = calloc(WordLengthMax + sizeof(size_t), sizeof(char));
        //size_t counter = 1;
        //memcpy((char*)value + WordLengthMax, &counter, sizeof(counter));
        //memcpy(value, key, WordLengthMax);
        //PushFront(list, value);
        //free(value);
    }
    else
    {
        (*((size_t*)((char*)HashTable->ValuesBucketArray[bucketIndex]->data + index * HashTable->ValuesBucketArray[bucketIndex]->elsize)))++;
        //void* NodeValue = (char*)list->data + index * list->elsize;//ListGetNodeValueInd(list, index);
        //size_t counter = 0;
        //memcpy(&counter, (char*)NodeValue + WordLengthMax, sizeof(size_t));
        //counter++;
        //memcpy((char*)NodeValue + WordLengthMax, &counter, sizeof(size_t));
        //ListUpdateNodeValue(list, NodeValue, index);
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

    size_t hash = SimpleHash(Key, WordLengthMax);

    size_t index = ListSearchInd(HashTable->KeysBucketArray[hash], Key, KeysComp);

    void* Value = (char*)HashTable->KeysBucketArray[hash]->data + index * HashTable->KeysBucketArray[hash]->elsize;// ListGetNodeValueInd(HashTable->BucketArray[hash], index);

    char string[WordLengthMax] = {};
    memcpy(string, Value, WordLengthMax);
    
    fprintf(stdout, "%s ", string);

    size_t Number =  *(size_t*)((char*)HashTable->ValuesBucketArray[hash]->data + index * HashTable->ValuesBucketArray[hash]->elsize);
    //memcpy(&Number, (char*)Value + WordLengthMax, sizeof(Number));

    fprintf(stdout, "%zu\n", Number);

    return MODULE_SUCCESS;
}

static int KeysComp(void* Key, void* KeyFromList)
{
    char* KeyStr = (char*)Key;
    char* KeyStrFromList = (char*)KeyFromList;

    return strcmp(KeyStr, KeyStrFromList);
}