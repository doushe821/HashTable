#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>

#include "FileBufferizer.h"
#include "HashFunctions.h"
#include "ErrorParser.h"
#include "HashTable.h"
#include "HashTableBench.h"
#include "ListStruct.h"
#include "CMDParser.h"
#include <x86intrin.h>

int main(int argc, char** argv)
{
    uint64_t start = 0;
    uint64_t end = 0;
    start = __rdtsc();
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

    crc32TableInit();

    switch(CMDFlags.searchOptimization)
    {
        case NO_OPTIMIZATIONS:
        {
            for(size_t i = 0; i < WordCounter - 1; i++)
            {
                HashTableInsertNaive(DataBuffer + i * sizeof(__m256), &HashTable);
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
                HashTableInsertNaive(DataBuffer + i * sizeof(__m256), &HashTable);
            }
            break;
        }
    }

    end = __rdtsc();

    fprintf(stderr, "Initialising ticks = %zu\n", end - start);

    SearchBench(&HashTable, CMDFlags);
    LoadFactorBench(&HashTable);
    HashTableDestr(&HashTable);
    free(DataBuffer);

    return 0;
}