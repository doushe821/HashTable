#include "../Headers/HashTableBench.h"
#include <inttypes.h>

#include <immintrin.h>

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

HashErrors SearchBench(HashTable_t* HashTable, flags_t Flags)
{
    FILE* SearchBenchFile = fopen("SearchBench.txt", "r+b");
    char* DataBuffer = NULL;
    volatile size_t ResultSum = 0;
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

    unsigned long long start = 0;
    unsigned long long end = 0;
    switch (Flags.searchOptimization)
    {
        case NO_OPTIMIZATIONS:
        {
            start = __rdtsc();
            for(size_t j = 0; j < 100; j++)
            {
                for(size_t i = 0; i < WordCounter; i++)
                {
                    ResultSum += HashTableSearchNaive(HashTable, (void*)(DataBuffer + i * 32));
                }
            }
            end = __rdtsc();
            break;
        }
        case SIMD_HASH:
        {
            start = __rdtsc();
            for(size_t j = 0; j < 100; j++)
            {
                for(size_t i = 0; i < WordCounter; i++)
                {
                    ResultSum += HashTableSearchSIMDHash(HashTable, (void*)(DataBuffer + i * 32));
                }
            }
            end = __rdtsc();
            break;
        }
        case SIMD_HASH_STRCMP_IN_ASM:
        {
            start = __rdtsc();
            for(size_t j = 0; j < 100; j++)
            {
                for(size_t i = 0; i < WordCounter; i++)
                {
                    ResultSum += HashTableSearchSIMDHashAsmStrcmp(HashTable, (void*)(DataBuffer + i * 32));
                }
            }
            end = __rdtsc();
            break;
        }
        case SIMD_HASH_ASM_SEARCH:
        {    
            start = __rdtsc();
            for(size_t j = 0; j < 100; j++)
            {
                for(size_t i = 0; i < WordCounter; i++)
                {
                    ResultSum += HashTableSearchAsmSearch(HashTable, (void*)(DataBuffer + i * 32));
                }
            }
            end = __rdtsc();
            break;    
        }
        default:
        {
            start = __rdtsc();
            for(size_t j = 0; j < 1; j++)
            {
                for(size_t i = 0; i < WordCounter; i++)
                    {
                        ResultSum += HashTableSearchNaive(HashTable, (void*)(DataBuffer + i * 32));
                    }
            }
            end = __rdtsc();
            break;
        }
    }

    fprintf(stderr, "Result sum = %zu\n", ResultSum);
    fprintf(stderr, "Ticks = %llu\n", end - start);

    fclose(SearchBenchFile);
    free(DataBuffer);
    return MODULE_SUCCESS;
}
