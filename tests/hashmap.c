#include "../src/hashmap.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

void insert_get() {
  int k_1 = 2;
  char *v_1 = "Hello";

  int k_2 = 8;
  char *v_2 = "How";

  int k_3 = 10;
  char *v_3 = "When";

  int k_4 = 65571;
  char *v_4 = "Long";

  int k_5 = 34;

  struct Table *map = hashmap();

  hashmap_insert(map, k_1, v_1);
  hashmap_insert(map, k_2, v_2);
  hashmap_insert(map, k_3, v_3);
  hashmap_insert(map, k_4, v_4);

  char *res1 = hashmap_get(map, k_1);
  char *res2 = hashmap_get(map, k_2);
  char *res3 = hashmap_get(map, k_3);
  char *res4 = hashmap_get(map, k_4);
  char *res5 = hashmap_get(map, k_5);

  assert(strcmp(res1, "Hello") == 0);
  assert(strcmp(res2, "How") == 0);
  assert(strcmp(res3, "When") == 0);
  assert(strcmp(res4, "Long") == 0);

  assert(res5 == NULL);
  printf("NULL\n");

  assert(strcmp(res2, "Hello") != 0);

  printf("%s\n", res1);
  printf("%s\n", res2);
  printf("%s\n", res3);
  printf("%s\n", res4);

  return;
}

int main() {
  insert_get();
  return 0;
}
