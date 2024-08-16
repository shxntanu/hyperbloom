#include <assert.h>
#include <errno.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

typedef struct BloomFilter {
  uint64_t *bv;            // Bit vector
  uint64_t size;           // Size of bit vector. Must be a power of 2.
  int hf;                  // Number of hash functions
  pthread_rwlock_t rwlock; // Centralized mutex
} BloomFilter;

BloomFilter *NewBloomFilter(uint64_t size, int hf);
void DestroyBloomFilter(BloomFilter *bf);
int setBit(BloomFilter *bf, uint64_t idx);
bool getBit(BloomFilter *bf, uint64_t idx);
bool Lookup(BloomFilter *bf, const char *entry);
int Insert(BloomFilter *bf, const char *entry);