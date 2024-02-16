#ifndef HASHTABLE_H
#define HASHTABLE_H

#include <stdlib.h>
#include "arena.h"

typedef struct HashTable HashTable;
typedef struct hashtable_operations hashtable_operations;

struct HashTable {
  size_t private_size;
  size_t entries;
  void** H;
  const hashtable_operations* methods;
  Arena* arena;
  };

struct hashtable_operations {
  void (*insert) (HashTable* self, char key[], size_t key_length, void* value);
  void* (*get) (HashTable* self, char key[], size_t key_length);
  void (*remove) (HashTable* self, char key[], size_t key_length);
};


void hashtable_init(HashTable* self, size_t hashtable_size, Arena* arena);

#endif
