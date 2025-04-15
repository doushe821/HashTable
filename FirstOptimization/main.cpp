#include <stdio.h>
#include <stdlib.h>
#include <x86intrin.h>

#include "TextPreprocessor.h"
#include "FileBufferizer.h"
#include "ErrorParser.h"
#include "HashTable.h"

//#include "hash.h"
//#include "List/List.h"

int main(int argc, char** argv)
{
    char A[64] = "perspective";
    char B[64] = "problematic";
    unsigned long long start, end;
    start = __rdtsc();
    for(int i = 0; i < 1000; i++)
    {
        //fprintf(stderr, "%d\n", strcmp64byte(A, B));
        strcmp64byte(A, B);
    }
    end = __rdtsc();
    fprintf(stderr, "%llu\n", end - start);
    start = __rdtsc();
    for(int i = 0; i < 1000; i++)
    {
        strncmp(A, B, 64);
    }
    end = __rdtsc();
    fprintf(stderr, "%llu\n", end - start);

    return 0;
    //unsigned long long start = __rdtsc();

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

    fprintf(stderr, "Hash Initialization Complete\n");

    fclose(HashTableFile);
    
    HashTableDump(&HashTable);

    const size_t TestWordsNumber = 71;
    char TestArray[TestWordsNumber][64] = 
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

    //unsigned long long end = __rdtsc();

    fprintf(stderr, "%llu\n", end - start);

    return 0;
}