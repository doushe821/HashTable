#include <stdio.h>
#include <stdlib.h>

#include "FileBufferizer.h"
#include "ErrorParser.h"
#include "HashTable.h"
#include "HashTableBench.h"
#include "List/ListStruct.h"
#include "x86intrin.h"

// TODO Better hash, bigger table, load factor, fix README, strcmp in asm, 

int main(int argc, char** argv)
{
    FILE* HashTableFile = fopen("ReadyTohash.txt", "r+b");
    
   if(HashTableFile == NULL)
   {
       return FILE_NULL_POINTER;
   }

   const size_t BucketCount = 1024;
   

   size_t WordCounter = 0;
   char* DataBuffer = FileToHashTableWordArray(HashTableFile, &WordCounter);
   fclose(HashTableFile);

    HashTable_t HashTable = HashTableInit(BucketCount);
    ParseError(HashTable.ErrorCode);

    for(size_t i = 0; i < WordCounter - 1; i++)
    {
        HashTableInsert(DataBuffer + i * sizeof(__m256), &HashTable);
    }

    SearchBench(&HashTable);
    LoadFactorBench(&HashTable);
    HashTableDestr(&HashTable);
    free(DataBuffer);

    return 0;
}