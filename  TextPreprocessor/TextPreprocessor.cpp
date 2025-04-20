#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "TextPreprocessor.h"
#include "ErrorParser.h"
#include "FileBufferizer.h"

int PreprocessText(FILE* InputFile, FILE* OutputFile)
{
    if(InputFile == NULL)
    {
        return FILE_NULL_POINTER;
    }

    if(OutputFile == NULL)
    {
        return FILE_NULL_POINTER;
    }

    size_t FileSize = GetFileSize(InputFile);
    if(FileSize == 0)
    {
        fprintf(stderr, "WARNING: input file size is zero (file is empty) %s:%d\n", __FILE__, __LINE__);
        return 0;
    }

    char* Buffer = (char*)calloc(FileSize, sizeof(char));
    char* OutputBuffer = (char*)calloc(FileSize, sizeof(char));

    if(fread(Buffer, sizeof(char), FileSize, InputFile) != FileSize)
    {
        return FREAD_ERROR;
    }

    
    size_t CharsWritten = 0;
    bool EndFile = 0;
    for(size_t CharsRead = 0; CharsRead < FileSize; CharsRead++)
    {
        while(!isalpha(Buffer[CharsRead]))
        {
            CharsRead++;
            if(CharsRead >= FileSize)
            {
                EndFile = 1;
                break;
            }
        }

        if(EndFile == 1)
        {
            break;
        }
        
        size_t WordSize = 0;
        while(isalpha(Buffer[CharsRead + WordSize]))
        {
            WordSize++;
        }
        //char* WordEnd = strchr(Buffer + CharsRead, ' ');
        //size_t WordSize = (size_t)WordEnd - (size_t)(Buffer + CharsRead);

        strncpy(OutputBuffer + CharsWritten, Buffer + CharsRead, WordSize);
        CharsWritten += WordSize;
        OutputBuffer[CharsWritten] = '\n';
        CharsWritten++;
        CharsRead += WordSize; 
    }

    fwrite(OutputBuffer, sizeof(char), CharsWritten, OutputFile);

    free(Buffer);
    free(OutputBuffer);
    
    return MODULE_SUCCESS;
}

