#include <stdio.h>
#include <stdlib.h>

#include "TextPreprocessor.h"
#include "FileBufferizer.h"
#include "ErrorParser.h"
#include "HashTable.h"
#include "List/ListStruct.h"
#include "x86intrin.h"
//#include "hash.h"
//#include "List/List.h"

int main(int argc, char** argv)
{

    unsigned long long start = __rdtsc();

    FILE* InputFP = fopen("HolyBible.txt", "r+b");
    if(InputFP == NULL)
    {
        fprintf(stderr, "Failed to open file: %s%d\n", __FILE__, __LINE__);
        return FILE_NULL_POINTER;
    }

    FILE* OutputFP = fopen("ReadyTohash.txt", "w+b");
    if(InputFP == NULL)
    {
        fprintf(stderr, "Failed to open file: %s%d\n", __FILE__, __LINE__);
        fclose(InputFP);
        return FILE_NULL_POINTER;
    }

    PreprocessText(InputFP, OutputFP);
    fclose(InputFP);
    fclose(OutputFP);

    FILE* HashTableFile = fopen("ReadyTohash.txt", "r+b");
    if(HashTableFile == NULL)
    {
        return FILE_NULL_POINTER;
    }

    HashTable_t HashTable = HashTableInit(HashTableFile);
    fprintf(stderr, "Hashing complete!\n");

    fclose(HashTableFile);
    
    HashTableDump(&HashTable);

    const size_t TestWordsNumber = 71;
    __attribute__((aligned(sizeof(__m256)))) char TestArray[TestWordsNumber][32] = 
    {
        "perspective",
        "permissions",
        "permissions",
        "programming",
        "performance",
        "programmers",
        "politicians",
        "predominant",
        "propagation",
        "picoseconds",
        "parallelism",
        "probability",
        "parentheses",
        "partitioned",
        "proprietary",
        "problematic",
        "practically",
        "propagating",
        "palindromes",
        "pictorially",
        "predictions",
        "potentially",
        "progression",
        "photographs",
        "positioning",
        "publication",
        "prototyping",
        "predictable",
        "permissible",
        "particulary",
        "permanently",
        "participate",
        "portability",
        "possibility",
        "productions",
        "prohibition",
        "precompiled",
        "programming",
        "performance",
        "programmers",
        "politicians",
        "predominant",
        "propagation",
        "picoseconds",
        "parallelism",
        "probability",
        "parentheses",
        "partitioned",
        "proprietary",
        "problematic",
        "practically",
        "propagating",
        "palindromes",
        "pictorially",
        "predictions",
        "potentially",
        "progression",
        "photographs",
        "positioning",
        "publication",
        "prototyping",
        "predictable",
        "permissible",
        "particulary",
        "permanently",
        "participate",
        "portability",
        "possibility",
        "productions",
        "prohibition",
        "precompiled",
    };    
    for(size_t i = 0; i < TestWordsNumber; i++)
    {
        HashTableSearch(&HashTable, TestArray[i]);
    }

    
    HashTableDestr(&HashTable);

    unsigned long long end = __rdtsc();

    fprintf(stderr, "%llu\n", end - start);

    return 0;
}