#include <string.h>
#include <stdio.h>

#include "hash.h"

size_t SimpleHash(void* value, size_t size)
{
    const char seed = 69;
    const int TableWidth = 256;
    size_t HashSum = 0;
    for(size_t i = 0; i < size; i++)
    {
        HashSum += (*((char*)value) ^ seed);
    }
    return HashSum % TableWidth;

}