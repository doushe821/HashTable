#include <stdio.h>
#include <stdlib.h>

#include "TextPreprocessor.h"
#include "FileBufferizer.h"
#include "ErrorParser.h"
#include "HashTable.h"
//#include "hash.h"
//#include "List/List.h"

int main(int argc, char** argv)
{
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

    printf("%d\n", PreprocessText(InputFP, OutputFP));
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

    HashTableDestr(&HashTable);

    return 0;
}