#include "HashFunctions.h"
#include <stdlib.h>
#include <xmmintrin.h>
#include <emmintrin.h>
#include <x86intrin.h>
#include <inttypes.h>
#include <assert.h>

size_t MurmurHash2 (char * key, size_t len, size_t MaxValue)
{
    size_t m = 0x5bd1e995;
    size_t seed = 0;
    size_t r = 24;

    size_t h = seed ^ len;

    const unsigned char * data = (const unsigned char *)key;
    unsigned int k = 0;

    while (len >= 4)
    {
        k  = data[0];
        k |= data[1] << (unsigned int)(8);
        k |= data[2] << (unsigned int)(16);
        k |= data[3] << (unsigned int)(24);

        k *= (unsigned)m;
        k ^= k >> (unsigned)r;
        k *= (unsigned)m;

        h *= (unsigned)m;
        h ^= k;

        data += (unsigned)4;
        len -= (size_t)4;
    }

    switch (len)
    {
        case 3:
        {
            h ^= data[2] << 16;
            h ^= data[1] << 8;
            h ^= data[0];
            h *= m;
            break;
        }
        case 2:
        {
            h ^= data[1] << 8;
            h ^= data[0];
            h *= m;
            break;
        }
        case 1:
        {
            h ^= data[0];
            h *= m;
            break;
        }
        default:
        {
            break;
        }
    }

    h ^= h >> 13;
    h *= m;
    h ^= h >> 15;

    return h % MaxValue;
}

void crc32TableInit(uint32_t* Table)
{
    uint32_t crc = 0;
    for (int i = 0; i < 256; i++)
    {
        crc = i;
        for (int j = 0; j < 8; j++) 
        {
            if (crc & 1)
            {
                crc = (crc >> 1) ^ 0xEDB88320;
            }
            else
            {
                crc >>= 1;
            }
        }
        Table[i] = crc;
    }
}

uint32_t crc32Hash(void* Key, size_t Len, size_t MaxValue, uint32_t* crcTable)
{
    uint32_t crc = 0xFFFFFFFF;
    for (size_t i = 0; i < Len; i++)
    {
        uint8_t index = (crc ^ (*((char*)Key + i))) & 0xFF;
        crc = (crc >> 8) ^ crcTable[index];
    }
    return (crc ^ 0xFFFFFFFF) % (uint32_t)MaxValue;
}

uint64_t crc32HashIntrinsics(void* Key, size_t MaxValue)
{
    uint64_t crc = 0xFFFFFFFF;

    uint64_t part1 = *(const uint64_t*)(Key); // TODO strict aliasing (UB)
    crc = _mm_crc32_u64(crc, part1);

    uint64_t part2 = *(const uint64_t*)((char*)Key + 8);
    crc = _mm_crc32_u64(crc, part2);

    uint64_t part3 = *(const uint64_t*)((char*)Key + 16);
    crc = _mm_crc32_u64(crc, part3);

    uint64_t part4 = *(const uint64_t*)((char*)Key + 24);
    crc = _mm_crc32_u64(crc, part4);

    return (crc ^ 0xFFFFFFFF) % (uint32_t)MaxValue;
}