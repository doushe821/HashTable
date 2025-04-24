#include "HashTableBench.h"

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

HashErrors SearchBench(HashTable_t* HashTable)
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
    for(size_t i = 0; i < WordCounter; i++)
    {
        ResultSum += HashTableSearch(HashTable, (void*)(DataBuffer + i * 32));
    }

    fprintf(stderr, "Result sum = %zu\n", ResultSum);

    fclose(SearchBenchFile);
    free(DataBuffer);
    return MODULE_SUCCESS;
}
