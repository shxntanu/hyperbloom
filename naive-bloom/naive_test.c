#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "naive.h"

int main() {
  printf("Running tests...\n");
  TestBFSetByte();
  TestNewBloomFilter();
  TestBloomFilter();
  printf("All tests passed!\n");
  return 0;
}

void TestBFSetByte() {
  BloomFilter *bf = NewBloomFilter(1048576, 4);
  assert(bf != NULL, "NewBloomFilter should not return NULL");

  int err = setByte(bf, 100);
  assert(err == 0, "setByte should not return an error");

  bool bitExists = getByte(bf, 100);
  assert(bitExists, "Byte 100 should be set");

  bool falseBit = getByte(bf, 1048575);
  assert(!falseBit, "Byte 1048575 should not be set");

  DestroyBloomFilter(bf);
  printf("TestBFSetBit passed\n");
}

void TestNewBloomFilter() {
  BloomFilter *bf = NewBloomFilter(100000, 4);
  assert(bf == NULL, "NewBloomFilter should return NULL for invalid size");

  bf = NewBloomFilter(1048576, 4);
  assert(bf != NULL, "NewBloomFilter should not return NULL for valid size");

  DestroyBloomFilter(bf);
  printf("TestNewBloomFilter passed\n");
}

void TestBloomFilter() {
  BloomFilter *bf = NewBloomFilter(1048576, 4);
  assert(bf != NULL, "NewBloomFilter should not return NULL");

  const char *e1 = "b99afb65c9f97b2e0feea844eea55f69";
  const char *e2 = "f530e3093a1617d64f400c5578005b7c";
  const char *e3 = "b29317ac342ceafc79e59996678efeb3";
  const char *e4 = "00421829519ccc2834eedc2bac21df68";
  const char *fake1 = "hahaidontexist";
  const char *fake2 = "foobar";
  const char *fake3 = "turnips";
  const char *fake4 = "lavacakes";

  assert(Insert(bf, e1) == 0, "Insert should not return an error");
  assert(Insert(bf, e2) == 0, "Insert should not return an error");
  assert(Insert(bf, e3) == 0, "Insert should not return an error");
  assert(Insert(bf, e4) == 0, "Insert should not return an error");

  assert(Lookup(bf, e1), "e1 should exist in the filter");
  assert(Lookup(bf, e2), "e2 should exist in the filter");
  assert(Lookup(bf, e3), "e3 should exist in the filter");
  assert(Lookup(bf, e4), "e4 should exist in the filter");

  assert(!Lookup(bf, fake1), "fake1 should not exist in the filter");
  assert(!Lookup(bf, fake2), "fake2 should not exist in the filter");
  assert(!Lookup(bf, fake3), "fake3 should not exist in the filter");
  assert(!Lookup(bf, fake4), "fake4 should not exist in the filter");

  DestroyBloomFilter(bf);
  printf("TestBloomFilter passed\n");
}