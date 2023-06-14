#include "list.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct list {
  size_t capacity;
  size_t size;
  void **data;
  free_func_t freer;
} list_t;

// typedef void (*free_func_t)(void *);

list_t *list_init(size_t initial_size, free_func_t freer) {
  list_t *list = malloc(sizeof(list_t));
  assert(list != NULL);
  list->capacity = initial_size;
  list->size = 0;
  list->data = malloc(sizeof(void *) * initial_size);
  list->freer = freer;
  return list;
}

void list_free(list_t *list) {
  if (list->freer != NULL) {
    for (size_t i = 0; i < list->size; i++) {
      list->freer(list->data[i]);
    }
  }
  free(list->data);
  free(list);
}

size_t list_size(list_t *list) {
  assert(list != NULL);
  return list->size;
}

void *list_get(list_t *list, size_t index) {
  assert(index < list->size);
  assert(list->data[index] != NULL);
  return (list->data[index]);
}

void *list_remove(list_t *list, size_t index) {
  void *temp = list_get(list, index);
  for (int i = index; i < list->size - 1; i++) {
    list->data[i] = list->data[i + 1];
  }
  list->size--;
  return temp;
}

void list_add(list_t *list, void *value) {
  assert(value != NULL);
  if (list->size >= list->capacity) {
    list->data =
        (void **)realloc(list->data, list->capacity * 2 * sizeof(void *));
    list->capacity *= 2;
  }
  list->data[list->size] = value;
  list->size++;
}
