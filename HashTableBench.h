#ifndef HASH_TABLE_BENCH_INCLUDED
#define HASH_TABLE_BENCH_INCLUDED

#include "HashTable.h"
#include "CMDParser.h"

HashErrors LoadFactorBench(HashTable_t* HashTable);
HashErrors SearchBench(HashTable_t* HashTable, flags_t Flags, uint32_t* crc32table);

#endif