// A chaining hashmap implementation using the FNV-1a algorithm

#ifndef HASHMAP_H

#define HASHMAP_H

#include <stdint.h>
#include <stdlib.h>

// 32 because it's a power of 2
#define TABLE_CAPACITY (32)

// FNV-1a constants
#define FNV_PRIME (16777619)
#define FNV_OFFSET_BASIS (2166136261)

// We implement the hashmap for a integer key
// and string value only
struct Node {
  int key;
  char *val;
  // TODO: make use of this
  struct Node *next;
};

// container to hold (key, value) pairs
struct Table {
  uint32_t len;       // number of elements in the table
  uint32_t capacity;  // maximum capacity allowed by the table
  struct Node *inner; // array of nodes
};

// FNV-1a (Alternate) hashing algorithm
// Learn more: http://www.isthe.com/chongo/tech/comp/fnv/#FNV-1a
uint32_t hashmap_hash(int key, uint32_t capacity) {
  uint32_t hash = FNV_OFFSET_BASIS;
  hash = hash ^ key;
  hash = hash * FNV_PRIME;
  return hash % capacity;
}

struct Table *hashmap() {
  struct Table *map = (struct Table *)malloc(sizeof(struct Table));

  if (NULL == map)
    return NULL;

  map->len = 0;
  map->capacity = TABLE_CAPACITY;

  map->inner = (struct Node *)calloc(TABLE_CAPACITY, sizeof(struct Node));

  return map;
}

// TODO: use chaining
int hashmap_insert(struct Table *table, int key, char *value) {
  if (NULL == value)
    return -1;

  struct Node *temp = (struct Node *)malloc(sizeof(struct Node));
  temp->key = key;
  temp->val = value;

  // hash the keys to get an index into our cotainer
  int index = hashmap_hash(key, table->capacity);

  // if (0 != (table->inner + index)) {
  //   struct Node *iter = table->inner;
  //   while (iter->next) {
  //     iter = iter->next;
  //   }
  //   iter->next = temp;

  //   return 0;
  // }

  table->inner[index] = *temp;
  table->len++;

  return 0;
}

// TODO: use chaining to get the val incase collision
char *hashmap_get(struct Table *table, int key) {
  int index = hashmap_hash(key, table->capacity);

  // TODO: remove this as this particular case if highly improbable
  if (index >= table->capacity)
    return NULL;

  if (NULL == table->inner + index)
    return NULL;

  return table->inner[index].val;
}

// TODO: use chaining
char *hashmap_remove(struct Table *table, int key) {
  char *val = hashmap_get(table, key);

  int index = hashmap_hash(key, table->capacity);

  free(table->inner + index);
  table->len--;
  return val;
}

#endif
