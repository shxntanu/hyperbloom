#include <assert.h>
#include <errno.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/**
 * Macro to perform assertions.
 */
#define assert(condition, message)                                             \
  do {                                                                         \
    if (!(condition)) {                                                        \
      fprintf(stderr, "Assertion failed: %s\n", message);                      \
      exit(1);                                                                 \
    }                                                                          \
  } while (0)

/**
 * BloomFilter is a bloomfilter backed by an array of unsigned 64 bit integers
 * (with bits encoded in each one). It uses central locking via a RWMutex and
 * supports both synchronous and asynchronous inserts and lookups.
 *
 * Note: Here, a bit-vector is used and not a byte-vector, i.e. every bit in the
 * filter is assigned to a bit.
 */
typedef struct BloomFilter {
  uint64_t *bv;  // Bit vector
  uint64_t size; // Size of bit vector. Must be a power of 2.
  int hf;        // Number of hash functions

  /**
   * Using a readers-writer lock here where in a multithreaded context, multiple
   * threads can gain access to the lock to read simultaneously. However, only a
   * single writer can write at any time and all readers wait for the lock to be
   * released by the writer before they can read. The writer also has to wait
   * until all active readers finish.
   */
  pthread_rwlock_t rwlock;
} BloomFilter;

/**
 * Create and return a pointer to a Bloom filter, given a size and number of
 * hash functions to apply.
 *
 * Parameters:
 * - `size`: the size (in bits) of the filter
 * - `hf`: number of hash functions to apply in the filter.
 */
BloomFilter *NewBloomFilter(uint64_t size, int hf);

/**
 * Manually free a Bloom filter after use in order to avoid memory leaks.
 */
void DestroyBloomFilter(BloomFilter *bf);

/**
 * Function to set a bit in the filter at a particular position.
 * Parameters:
 * - `bf`: Bloom filter
 * - `idx`: index at which the bit needs to be set.
 */
int setBit(BloomFilter *bf, uint64_t idx);

/**
 * Asynchronous function to set a bit in the filter at a particular position.
 * To be used in a single threaded context to avoid the mutex wait.
 *
 * Parameters:
 * - `bf`: Bloom filter
 * - `idx`: index at which the bit needs to be set.
 */
int setBitAsync(BloomFilter *bf, uint64_t idx);

/**
 * Function to read a bit at a position from the filter.
 *
 * Parameters:
 * - `bf`: Bloom filter
 * - `idx`: index from which the bit needs to be read.
 */
bool getBit(BloomFilter *bf, uint64_t idx);

/**
 * Asynchronous function to read a bit at a position from the filter.
 * To be used in a single threaded context to avoid the
 * mutex wait.
 *
 * Parameters:
 * - `bf`: Bloom filter
 * - `idx`: index from which the bit needs to be read.
 */
bool getBitAsync(BloomFilter *bf, uint64_t idx);

/**
 * Looks up an entry into the NaiveBloomFilter. Returns true if a match is
 * found, false otherwise. If an error occurs, will also return false. This
 * perform a reader lock on the filter (writers must wait until all active
 * readers finish).
 *
 * Parameters:
 * - `bf`: Bloom filter
 * - `entry`: string that needs to be added to the bloom filter
 */
bool Lookup(BloomFilter *bf, const char *entry);

/**
 * Inserts an entry into the filter.
 * This performs a writer lock on the filter (all readers must wait until
 * the active writer finishes and releases the lock).
 */
int Insert(BloomFilter *bf, const char *entry);

/**
 * Flushes the Bloom filter to a file.
 */
int Write(BloomFilter *bf, const char *filename);

/**
 * Reads an existing Bloom filter from a file.
 */
BloomFilter *Load(const char *filename);

/**
 * Function to merge a loaded BloomFilter with an existing one
 */
int MergeBloomFilter(BloomFilter *bf, const char *filename);

/**
 * Testing functions to verify intended functionality.
 */
void TestBFSetBit();
void TestNewBloomFilter();
void TestBloomFilter();