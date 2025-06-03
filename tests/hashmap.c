#include "../src/hashmap.h"
#include <assert.h>
#include <string.h>

void insert_get() {
  int k_1 = 2;
  char *v_1 = "Hello";

  int k_2 = 8;
  char *v_2 = "How";

  int k_3 = 10;
  char *v_3 = "When";

  struct Table *map = hashmap();

  hashmap_insert(map, k_1, v_1);
  hashmap_insert(map, k_2, v_2);
  hashmap_insert(map, k_3, v_3);

  char *res1 = hashmap_get(map, k_1);
  char *res2 = hashmap_get(map, k_2);
  char *res3 = hashmap_get(map, k_3);

  assert(strcmp(res1, "Hello") == 0);
  assert(strcmp(res2, "How") == 0);
  assert(strcmp(res3, "When") == 0);

  assert(strcmp(res2, "Hello") != 0);

  return;
}

int main() {
  insert_get();
  return 0;
}
