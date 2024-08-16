#include "bloom.h"
#include "xxhash.h"

BloomFilter *NewBloomFilter(uint64_t size, int hf) {
  if (size < 64) {
    fprintf(stderr, "Filter size must be at least 64\n");
    return NULL;
  }
  if (size & (size - 1) != 0) {
    fprintf(stderr, "Filter must be a power of 2\n");
    return NULL;
  }

  BloomFilter *bf = (BloomFilter *)malloc(sizeof(BloomFilter));
  if (!bf) {
    perror("Failed to allocate bloom filter.");
    return NULL;
  }

  bf->size = size;
  bf->hf = hf;
  bf->bv = calloc(size / 64, sizeof(uint64_t));

  if (!bf->bv) {
    perror("Failed to allocate bit vector.");
    return NULL;
  }

  if (pthread_rwlock_init(&bf->rwlock, NULL) != 0) {
    perror("Failed to initialize rwlock");
    free(bf->bv);
    free(bf);
    return NULL;
  }

  return bf;
}

void DestroyBloomFilter(BloomFilter *bf) {
  if (bf) {
    pthread_rwlock_destroy(&bf->rwlock);
    free(bf->bv);
    free(bf);
  }
}

int setBit(BloomFilter *bf, uint64_t idx) {
  if (idx > bf->size - 1) {
    perror("Index cannot be larger than filter size");
    return -1;
  }

  uint64_t intID = idx / 64;
  uint64_t bitID = idx & 63;
  pthread_rwlock_wrlock(&bf->rwlock);
  bf->bv[intID] |= (1 << bitID);
  pthread_rwlock_unlock(&bf->rwlock);
  return 0;
}

int setBitAsync(BloomFilter *bf, uint64_t idx) {
  if (idx >= bf->size) {
    fprintf(stderr, "Index can't be larger than filter size\n");
    return -1;
  }
  uint64_t intID = idx / 64;
  uint64_t bitID = idx & 63;

  // No locking here
  bf->bv[intID] |= (1ULL << bitID);
  return 0;
}

bool getBit(BloomFilter *bf, uint64_t idx) {
  if (idx > bf->size - 1) {
    perror("Index cannot be larger than filter size");
    return -1;
  }

  uint64_t intID = idx / 64;
  uint64_t bitID = idx & 63;
  pthread_rwlock_rdlock(&bf->rwlock);
  bool exists = bf->bv[bitID] & (1ULL << bitID) != 0;
  pthread_rwlock_unlock(&bf->rwlock);
  return exists;
}

bool getBitAsync(BloomFilter *bf, uint64_t idx) {
  if (idx >= bf->size) {
    fprintf(stderr, "Index can't be larger than filter size\n");
    return false;
  }
  uint64_t intID = idx / 64;
  uint64_t bitID = idx & 63;

  // No locking here
  return (bf->bv[intID] & (1ULL << bitID)) != 0;
}

bool Lookup(BloomFilter *bf, const char *entry) {}