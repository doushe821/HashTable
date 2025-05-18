#include "HashTableBench.h"
#include <inttypes.h>

HashErrors LoadFactorBench(HashTable_t* HashTable)
{
    FILE* LoadFactorTest = fopen("LoadFactor.csv", "w+b");
    if(LoadFactorTest == NULL)
    {
        fprintf(stderr, "Couldn't open Load Factor Test file, skipping test\n");
        return MODULE_SUCCESS;
    }
    else
    {
        for(size_t i = 0; i < HashTable->BucketCount; i++)
        {
            if(HashTable->KeysBucketArray[i] != NULL)
            {
                fprintf(LoadFactorTest, "%zu, %zu\n", i, GetLinearListSize(HashTable->KeysBucketArray[i]) / 32);
            }
            else
            {
                fprintf(LoadFactorTest, "%zu, %d\n", i, 0);
            }
        }
    }
    return MODULE_SUCCESS;
}

HashErrors SearchBench(HashTable_t* HashTable, flags_t Flags, uint32_t* crc32table)
{
    FILE* SearchBenchFile = fopen("SearchBench.txt", "r+b");
    char* DataBuffer = NULL;
    size_t ResultSum = 0;
    size_t WordCounter = 0;
    if(SearchBenchFile == NULL)
    {
        fprintf(stderr, "Couldn't open search bench file, skipping test\n");
        return MODULE_SUCCESS;
    }
    else
    {
        DataBuffer = FileToHashTableWordArray(SearchBenchFile, &WordCounter);
    }
    fprintf(stderr, "Words in bench = %zu\n", WordCounter);

    switch (Flags.searchOptimization)
    {
        case NO_OPTIMIZATIONS:
        {
            for(size_t i = 0; i < WordCounter; i++)
            {
                ResultSum += HashTableSearchNaive(HashTable, (void*)(DataBuffer + i * 32), crc32table);
            }
            break;
        }
        case SIMD_HASH:
        {
            for(size_t i = 0; i < WordCounter; i++)
            {
                ResultSum += HashTableSearchSIMDHash(HashTable, (void*)(DataBuffer + i * 32));
            }
            break;
        }
        case SIMD_HASH_STRCMP_IN_ASM:
        {
            for(size_t i = 0; i < WordCounter; i++)
            {
                ResultSum += HashTableSearchSIMDHashAsmStrcmp(HashTable, (void*)(DataBuffer + i * 32));
            }
            break;
        }
        case SIMD_HASH_ASM_SEARCH:
        {
            for(size_t i = 0; i < WordCounter; i++)
            {
                ResultSum += HashTableSearchAsmSearch(HashTable, (void*)(DataBuffer + i * 32));
            }
            break;    
        }
        default:
        {
            for(size_t i = 0; i < WordCounter; i++)
            {
                ResultSum += HashTableSearchNaive(HashTable, (void*)(DataBuffer + i * 32), crc32table);
            }
            break;
        }
    }

    fclose(SearchBenchFile);
    free(DataBuffer);
    return MODULE_SUCCESS;
}
