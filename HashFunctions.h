#ifndef HASH_FUNCTIONS_H_INCLUDED
#define HASH_FUNCTIONS_H_INCLUDED

#include <inttypes.h>
#include <stdlib.h>
//
//size_t WhackyHash(void* Key, size_t len, size_t MaxValue);
//size_t WhackyHash2(void* Key, size_t len, size_t MaxValue);
size_t MurmurHash2 (char * key, size_t len, size_t MaxValue);
//size_t WhackyHash2_SIMD(void* Key, size_t len, size_t MaxValue);
uint32_t crc32Hash(void* Key, size_t Len, size_t MaxValue, uint32_t* crcTable);
uint64_t crc32HashIntrinsics(void* Key, size_t MaxValue);
void crc32TableInit(uint32_t* Table);

#endif
