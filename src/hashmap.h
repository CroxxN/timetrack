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
  uint32_t key;
  char *val;
  struct Node *next;
};

// container to hold (key, value) pairs
struct Table {
  uint32_t len;        // number of elements in the table
  uint32_t capacity;   // maximum capacity allowed by the table
  struct Node **inner; // array of nodes
};

// FNV-1a (Alternate) hashing algorithm
// Learn more: http://www.isthe.com/chongo/tech/comp/fnv/#FNV-1a
uint32_t hashmap_hash(uint32_t key, uint32_t capacity) {
  uint32_t hash = FNV_OFFSET_BASIS;
  int octet;
  for (int i = 0; i < 4; i++) {
    octet = (key >> (8 * i)) & 0xFF;
    hash = hash ^ octet;
    hash = hash * FNV_PRIME;
  }
  return hash % capacity;
}

struct Table *hashmap(void) {
  struct Table *map = (struct Table *)malloc(sizeof(struct Table));

  if (NULL == map)
    return NULL;

  map->len = 0;
  map->capacity = TABLE_CAPACITY;

  map->inner = (struct Node **)calloc(TABLE_CAPACITY, sizeof(struct Node));

  for (int i = 0; i < TABLE_CAPACITY; i++) {
    map->inner[i] = NULL;
  }

  return map;
}

int hashmap_insert(struct Table *table, uint32_t key, char *value) {
  if (NULL == value)
    return -1;

  struct Node *temp = (struct Node *)malloc(sizeof(struct Node));
  temp->key = key;
  temp->val = value;

  // hash the keys to get an index into our cotainer
  int index = hashmap_hash(key, table->capacity);

  if (NULL != (table->inner[index])) {
    struct Node *iter = table->inner[index];
    if (iter->key == key) {
      iter->val = value;
      return 0;
    }
    while (iter->next) {
      if (iter->key == key) {
        iter->val = value;
        return 0;
      }
      iter = iter->next;
    }
    iter->next = temp;

    return 0;
  }

  // MARKER:
  // table->inner[index] = *temp;
  table->inner[index] = temp;
  table->len++;

  return 0;
}

char *hashmap_get(struct Table *table, uint32_t key) {
  int index = hashmap_hash(key, table->capacity);

  if (NULL == table->inner[index])
    return NULL;

  if (table->inner[index]->key != key) {
    struct Node *iter = table->inner[index];

    while (iter) {
      if (iter->key == key)
        return iter->val;
      iter = iter->next;
    }
    return NULL;
  }

  return table->inner[index]->val;
}

char *hashmap_remove(struct Table *table, uint32_t key) {
  char *val = NULL;

  int index = hashmap_hash(key, table->capacity);

  if (NULL == table->inner[index])
    return NULL;

  if (table->inner[index]->key == key) {
    val = table->inner[index]->val;
    if (!table->inner[index]->next) {
      free(table->inner[index]);
      table->inner[index] = (struct Node *)calloc(1, sizeof(struct Node));
    } else
      *table->inner[index] = *table->inner[index]->next;
  } else {

    struct Node *iter = table->inner[index];
    struct Node *prev = table->inner[index];

    while (iter) {
      if (iter->key == key) {
        val = iter->val;
        prev->next = iter->next;
        free(iter);
        break;
      }
      prev = iter;
      iter = iter->next;
    }
  }

  table->len--;
  return val;
}

#endif
