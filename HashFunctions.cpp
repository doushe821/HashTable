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

    uint64_t part1 = *(const uint64_t*)(Key);
    crc = _mm_crc32_u64(crc, part1);

    uint64_t part2 = *(const uint64_t*)((char*)Key + 8);
    crc = _mm_crc32_u64(crc, part2);

    uint64_t part3 = *(const uint64_t*)((char*)Key + 16);
    crc = _mm_crc32_u64(crc, part3);

    uint64_t part4 = *(const uint64_t*)((char*)Key + 24);
    crc = _mm_crc32_u64(crc, part4);

    return (crc ^ 0xFFFFFFFF) % (uint32_t)MaxValue;
}

//size_t WhackyHash(void* Key, size_t len, size_t MaxValue)
//{
//    size_t semen = 0x69696ABE1;
//    size_t MLG = 0xABE1;
//    size_t tony = 0xEDA666;
//    
//    size_t HashSum = 0;
//    for(size_t i = 0; i < len / 4; i++)
//    {
//        int HashSubSum = ((char*)Key)[i * 4] +  ((char*)Key)[i * 4 + 1] + ((char*)Key)[i * 4 + 2] + ((char*)Key)[i * 4 + 3];
//        HashSum += ((HashSubSum ^ semen) * MLG);
//        HashSum ^= semen;
//        HashSum *= MLG;
//    }
//
//    return HashSum % MaxValue;
//}
//
//size_t WhackyHash2(void* Key, size_t len, size_t MaxValue)
//{
//    size_t seed = 0x69696ABE1;
//    
//    size_t HashSum = 0;
//    for(size_t i = 0; i < len / 4; i++)
//    {
//        int HashSubSum = ((char*)Key)[i * 4] +  ((char*)Key)[i * 4 + 1] + ((char*)Key)[i * 4 + 2] + ((char*)Key)[i * 4 + 3];
//        HashSum += (HashSubSum ^ seed);
//    }
//    return HashSum % MaxValue;
//}
//
//size_t WhackyHash2_SIMD(void* Key, size_t len, size_t MaxValue)
//{
//    size_t seed = 0x69696ABE1;
//    
//    __m256i seedm256 = _mm256_set1_epi64x(seed);
//
//    size_t HashSubSums[4] = {};
//    for(size_t i = 0; i < 4; i++)
//    {
//        HashSubSums[i] = ((char*)Key)[i * 8] +  ((char*)Key)[i * 8 + 1] + ((char*)Key)[i * 8 + 2] + ((char*)Key)[i * 8 + 3]
//        + ((char*)Key)[i * 8 + 4] +  ((char*)Key)[i * 8 + 5] + ((char*)Key)[i * 8 + 6] + ((char*)Key)[i * 8 + 7];
//    }
//
//    __m256i SubSums256 = {(long long)HashSubSums[0], (long long)HashSubSums[1], (long long)HashSubSums[2], (long long)HashSubSums[3]};
//    SubSums256 = _mm256_xor_si256(SubSums256, seedm256);
//
//    size_t HashSum = SubSums256[0] + SubSums256[1] + SubSums256[2] + SubSums256[3];
//    return HashSum % MaxValue;
//}

//size_t crc32HashIntrinsics(const void* Key, const size_t MaxValue)
//{
//    size_t seed = 0xFFFFFFFFFFFFFFFF;
//
//    size_t partition1 = *(const size_t*)(Key);
//    seed = _mm_crc32_u64(seed, partition1);
//
//    size_t partition2 = *(const size_t*)(Key + 8);
//    seed = _mm_crc32_u64(seed, partition2);
//
//    return (seed ^ 0xFFFFFFFF) % MaxValue;
//}


//int crc32TableInit(uint32_t* table)
//{
//    uint8_t index = 0;
//    uint8_t z = 0;
//    do
//    {
//        table[index]=index;
//        for(z = 8; z > 0; z--) 
//        {
//            if(table[index] & 1)
//            {
//                table[index] = (table[index]>>1)^0xEDB88320;
//            }
//            else
//            {
//                table[index] >>= 1;
//            }
//        }
//    }while(++index);
//    return 0;
//}
//
//size_t crc32Hash(const void* Key, size_t len, size_t MaxValue, uint32_t* crcTable)
//{
//    u_int32_t crc = 0xFFFFFFFFFFFFFFFF;
//    for(size_t i = 0; i < len; i++)
//    {
//        u_int8_t index = (crc ^ *(u_int8_t*)(Key + i * sizeof(u_int8_t))) & 0xFF;
//        crc = (crc >> 8) ^ crcTable[i];
//    }
//    return (crc ^ 0xFFFFFFFF) % MaxValue;
//}
