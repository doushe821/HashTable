#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "HashTable.h"
#include "List/List.h"
#include "ErrorParser.h"
#include "hash.h"
#include "FileBufferizer.h"

const size_t HashTableWidth = 256; 
const size_t WordLengthMax = 64;
const size_t listInitSize = 4048;

static enum ErrorCodes ListInsertOrIncrementHashTable(char* key, List_t* list);

static int KeysComp(void* Key, void* KeyFromList);

HashTable_t HashTableInit(FILE* fp)
{
    char* buffer = FileToString(fp);
    size_t bufferSize = GetFileSize(fp);
    HashTable_t HashTable = {};

    List_t** BucketArray = (List_t**)calloc(sizeof(List_t*), HashTableWidth);
    if(BucketArray == NULL)
    {
        free(buffer);
        HashTable.ErrorCode = ALLOCATION_FAILURE;
        return HashTable;
    }

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
        
        if(BucketArray[hash] == NULL)
        {
            ListInit(&(BucketArray[hash]), listInitSize, sizeof(size_t) + WordLengthMax);
        }

        ParseError(ListInsertOrIncrementHashTable(word, BucketArray[hash])); 
    }
    HashTable = {BucketArray};
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
    NodeInfo info = {};
    size_t Number = 0;
    char string[WordLengthMax] = {};
    for(size_t i = 0; i < HashTableWidth; i++)
    {
        if(HashTable->BucketArray[i] != NULL)
        {
            size_t index = 0;
            info = LNodeInfo(HashTable->BucketArray[i], index);
            index = info.next;
            while(index != 0)
            {
                void* Value = ListGetNodeValueInd(HashTable->BucketArray[i], index);
    
                memcpy(string, Value, WordLengthMax);
    
                fprintf(dumpFile, "%s ", string);
    
                memcpy(&Number, (char*)Value + WordLengthMax, sizeof(Number));
                
                fprintf(dumpFile, "%zu\n", Number);

                info = LNodeInfo(HashTable->BucketArray[i], index);
                index = info.next;
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
        void* address = (List_t*)(HashTable->BucketArray[i]);
        if(address != NULL)
        {
            ListDstr((List_t*)address);
        }
    }
    free(HashTable->BucketArray);
    return MODULE_SUCCESS;
}

static enum ErrorCodes ListInsertOrIncrementHashTable(char* key, List_t* list)
{
    size_t index = ListSearchInd(list, key, KeysComp); 
    if(index == 0)
    {
        void* value = calloc(WordLengthMax + sizeof(size_t), sizeof(char));
        size_t counter = 1;
        memcpy((char*)value + WordLengthMax, &counter, sizeof(counter));
        memcpy(value, key, WordLengthMax);
        PushFront(list, value);
        free(value);
    }
    else
    {
        void* NodeValue = ListGetNodeValueInd(list, index);
        size_t counter = 0;
        memcpy(&counter, (char*)NodeValue + WordLengthMax, sizeof(size_t));
        counter++;
        memcpy((char*)NodeValue + WordLengthMax, &counter, sizeof(size_t));
        ListUpdateNodeValue(list, NodeValue, index);
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

    size_t index = ListSearchInd(HashTable->BucketArray[hash], Key, KeysComp);

    void* Value = ListGetNodeValueInd(HashTable->BucketArray[hash], index);

    char string[WordLengthMax] = {};
    memcpy(string, Value, WordLengthMax);
    
    fprintf(stdout, "%s ", string);

    size_t Number =  0;
    memcpy(&Number, (char*)Value + WordLengthMax, sizeof(Number));
    
    fprintf(stdout, "%zu\n", Number);


    return MODULE_SUCCESS;
}

static int KeysComp(void* Key, void* KeyFromList)
{
    char* KeyStr = (char*)Key;
    char* KeyStrFromList = (char*)KeyFromList;

    return strcmp64byte(KeyStr, KeyStrFromList);
}