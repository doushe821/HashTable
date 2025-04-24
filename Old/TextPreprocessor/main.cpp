#include <stdio.h>
#include <string.h>

#include "TextPreprocessor.h"
#include "FileBufferizer.h"

int main(int argc, char** argv)
{
    char FileName[FILENAME_MAX] = {};
    if(argc <= 1)
    {
        strcpy(FileName, "Text.txt");
    }
    else
    {
        strcpy(FileName, argv[1]);
    }

    FILE* fpIn = fopen(FileName, "r+b");
    if(fpIn == NULL)
    {
        fprintf(stderr, "Cannot open file.\n");
        return -1;
    }

    FILE* fpOut = fopen("ReadyToHash.txt", "w+b");

    if(fpOut == NULL)
    {
        fclose(fpIn);
        return -1;
    }

    PreprocessText(fpIn, fpOut);

    fclose(fpIn);
    fclose(fpOut);

    return 0;
}