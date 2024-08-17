#include "naive.h"
#include "hashing.h"
#include "xxhash.h"

BloomFilter *NewBloomFilter(uint64_t size, int hf) {
  if (size < 64) {
    fprintf(stderr, "Filter size must be at least 64\n");
    return NULL;
  }
  if ((size & (size - 1)) != 0) {
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
  bf->bv = calloc(size, sizeof(uint8_t));

  if (!bf->bv) {
    perror("Failed to allocate byte vector.");
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

int setByte(BloomFilter *bf, uint64_t idx) {
  if (idx > bf->size - 1) {
    perror("Index cannot be larger than filter size");
    return -1;
  }

  pthread_rwlock_wrlock(&bf->rwlock);
  bf->bv[idx] = 1;
  pthread_rwlock_unlock(&bf->rwlock);
  return 0;
}

int setByteAsync(BloomFilter *bf, uint64_t idx) {
  if (idx >= bf->size) {
    fprintf(stderr, "Index can't be larger than filter size\n");
    return -1;
  }

  // No locking here
  bf->bv[idx] = 1;
  return 0;
}

bool getByte(BloomFilter *bf, uint64_t idx) {
  if (idx > bf->size - 1) {
    perror("Index cannot be larger than filter size");
    return -1;
  }

  pthread_rwlock_rdlock(&bf->rwlock);
  switch (bf->bv[idx]) {
  case 1:
    pthread_rwlock_unlock(&bf->rwlock);
    return true;
  default:
    pthread_rwlock_unlock(&bf->rwlock);
    return false;
  }
}

bool getBitAsync(BloomFilter *bf, uint64_t idx) {
  if (idx >= bf->size) {
    fprintf(stderr, "Index can't be larger than filter size\n");
    return false;
  }

  switch (bf->bv[idx]) {
  case 1:
    pthread_rwlock_unlock(&bf->rwlock);
    return true;
  default:
    pthread_rwlock_unlock(&bf->rwlock);
    return false;
  }
}

bool Lookup(BloomFilter *bf, const char *entry) {
  uint64_t *hashes = hashEntry(entry, strlen(entry), bf->hf);
  for (int i = 0; i < bf->hf; i++) {
    uint64_t lookup_idx = hashes[i] & (bf->size - 1);
    if (!getByte(bf, lookup_idx)) {
      return false;
    }
  }
  return true;
}

int Insert(BloomFilter *bf, const char *entry) {
  uint64_t *hashes = hashEntry(entry, strlen(entry), bf->hf);
  for (int i = 0; i < bf->hf; i++) {
    uint64_t lookup_idx = hashes[i] & (bf->size - 1);
    if (setByte(bf, lookup_idx) != 0) {
      return -1;
    }
  }
  return 0;
}

int Write(BloomFilter *bf, const char *filename) {
  FILE *f = fopen(filename, "wb");
  if (f == NULL) {
    perror("Failed to open file for writing");
    return -1;
  }

  printf("Writing byte vector to file...\n");

  // Write the size and number of hash functions
  if (fwrite(&bf->size, sizeof(uint64_t), 1, f) != 1 ||
      fwrite(&bf->hf, sizeof(int), 1, f) != 1) {
    perror("Failed to write filter metadata");
    fclose(f);
    return -1;
  }

  // Write the byte vector
  size_t bv_size = bf->size;
  if (fwrite(bf->bv, sizeof(uint8_t), bv_size, f) != bv_size) {
    perror("Failed to write byte vector");
    fclose(f);
    return -1;
  }

  fclose(f);
  printf("Successfully wrote byte-vector to file: %s\n", filename);
  return 0;
}

BloomFilter *Load(const char *filename) {
  FILE *f = fopen(filename, "rb");
  if (f == NULL) {
    perror("Failed to open file for reading");
    return NULL;
  }

  uint64_t size;
  int hf;

  // Read the size and number of hash functions
  if (fread(&size, sizeof(uint64_t), 1, f) != 1 ||
      fread(&hf, sizeof(int), 1, f) != 1) {
    perror("Failed to read filter metadata");
    fclose(f);
    return NULL;
  }

  BloomFilter *bf = NewBloomFilter(size, hf);
  if (bf == NULL) {
    fclose(f);
    return NULL;
  }

  // Read the byte vector
  size_t bv_size = size;
  if (fread(bf->bv, sizeof(uint8_t), bv_size, f) != bv_size) {
    perror("Failed to read byte vector");
    DestroyBloomFilter(bf);
    fclose(f);
    return NULL;
  }

  fclose(f);
  printf("Loaded bitvector from file: %s\n", filename);
  return bf;
}

/**
 * Function to merge a loaded BloomFilter with an existing one
 */
int MergeBloomFilter(BloomFilter *bf, const char *filename) {
  BloomFilter *loaded_bf = Load(filename);
  if (loaded_bf == NULL) {
    return -1;
  }

  if (bf->size != loaded_bf->size || bf->hf != loaded_bf->hf) {
    fprintf(stderr, "Mismatch in BloomFilter parameters\n");
    DestroyBloomFilter(loaded_bf);
    return -1;
  }

  size_t bv_size = bf->size;
  pthread_rwlock_wrlock(&bf->rwlock);
  for (size_t i = 0; i < bv_size; i++) {
    bf->bv[i] |= loaded_bf->bv[i];
  }
  pthread_rwlock_unlock(&bf->rwlock);

  DestroyBloomFilter(loaded_bf);
  return 0;
}