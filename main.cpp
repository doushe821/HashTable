#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>

#include "FileBufferizer.h"
#include "HashFunctions.h"
#include "ErrorParser.h"
#include "HashTable.h"
#include "HashTableBench.h"
#include "List/ListStruct.h"
#include "CMDParser.h"
#include <x86intrin.h>

int main(int argc, char** argv)
{
    flags_t CMDFlags = {};

    ParseCMD(argc, argv, &CMDFlags);

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

    uint32_t crc32table[256] = {};
    crc32TableInit(crc32table);

    switch(CMDFlags.searchOptimization)
    {
        case NO_OPTIMIZATIONS:
        {
            for(size_t i = 0; i < WordCounter - 1; i++)
            {
                HashTableInsertNaive(DataBuffer + i * sizeof(__m256), &HashTable, crc32table);
            }
            break;
        }
        case SIMD_HASH:
        {
            for(size_t i = 0; i < WordCounter - 1; i++)
            {
                HashTableInsertSIMDHash(DataBuffer + i * sizeof(__m256), &HashTable);
            }
            break;  
        }
        case SIMD_HASH_STRCMP_IN_ASM:
        {
            for(size_t i = 0; i < WordCounter - 1; i++)
            {
                HashTableInsertSIMDHashAsmStrcmp(DataBuffer + i * sizeof(__m256), &HashTable);
            }
            break;
        }
        case SIMD_HASH_ASM_SEARCH:
        {
            for(size_t i = 0; i < WordCounter - 1; i++)
            {
                HashTableInsertSIMDHashAsmSearch(DataBuffer + i * sizeof(__m256), &HashTable);
            }
            break;
        }
        default:
        {
            for(size_t i = 0; i < WordCounter - 1; i++)
            {
                HashTableInsertNaive(DataBuffer + i * sizeof(__m256), &HashTable, crc32table);
            }
            break;
        }
    }


    SearchBench(&HashTable, CMDFlags, crc32table);
    LoadFactorBench(&HashTable);
    HashTableDestr(&HashTable);
    free(DataBuffer);

    return 0;
}