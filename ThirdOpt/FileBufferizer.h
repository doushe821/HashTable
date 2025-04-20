#ifndef FILE_BUFFERIZER_H_INCLUDED
#define FILE_BUFFERIZER_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>

char* FileToString(FILE* out);
size_t GetFileSize(FILE* fp);

#endif