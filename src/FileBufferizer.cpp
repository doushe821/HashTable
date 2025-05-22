#include "../Headers/FileBufferizer.h"

char* FileToString(FILE* out)
{
    if(out == NULL)
    {
        return NULL;
    }
    
    size_t size = GetFileSize(out);
    char* string = (char*)calloc(size + 1, sizeof(char));
    if(string == NULL)
    {
        return NULL;
    }
    string[size] = '\0';
    if(fread(string, sizeof(char), size, out) != size)
    {
        return NULL;
    }
    return string;
}

size_t GetFileSize(FILE* fp)
{
    
    fseek(fp, 0L, SEEK_END);
    
    size_t size = (size_t)ftell(fp);
    
    rewind(fp);
    return size;
}