#include "xxhash.h" // You'll need to download and include the xxHash library
#include <stdlib.h>
#include <string.h>

// Hash an entry "n" number of times with a 64 bit hash
uint64_t *hashEntry(const uint8_t *entry, size_t entry_len, int n) {
  uint64_t *out = malloc(n * sizeof(uint64_t));
  if (out == NULL) {
    perror("Failed to allocate memory for output array.");
    return NULL;
  }

  uint8_t *pert = malloc(entry_len);
  if (pert == NULL) {
    free(out);
    return NULL;
  }
  memcpy(pert, entry, entry_len);

  for (int i = 0; i < n; i++) {
    // Modify the last byte of pert
    pert[entry_len - 1] = pert[entry_len - 1] & 0xFF;

    // Hash the modified entry
    out[i] = XXH64(pert, entry_len, 0); // 0 is the seed
  }

  free(pert);
  return out;
}