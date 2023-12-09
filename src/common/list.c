#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

#include "list.h"

List *makeList(void *head, List *tail) {
  List *list = malloc(sizeof(List));
  //list->size = tail == NULL ? 1 : tail->size + 1;
  list->head = head;
  list->tail = tail;
  return list;
}

void freeList(List *list) {
  if (list->tail != NULL) {
    freeList(list->tail);
  }
  //free(list->head); // is this right?
  free(list);
}

void freeListWith(List *list, void (*freeValue)(void *)) {
  if (list->tail != NULL) {
    freeListWith(list->tail, freeValue);
  }
  freeValue(list->head);
  free(list);
}

/* Prints a list. Only works for char* list.
 */
void printList(List *list) {
  printf("[");
  while (list != NULL) {
    printf("%s", (char *)list->head);
    if (list->tail != NULL) {
      printf(", ");
    }
    list = list->tail;
  }
  printf("]\n");
}

int listSize(List *list) {
  int size = 0;
  while (list != NULL) {
    size++;
    list = list->tail;
  }
  return size;
}

void reverseList(List **list) {
  if (*list == NULL) return;

  List *prev = NULL;
  List *curr = *list;
  List *next = NULL;
  //size_t size = (*list)->size;
  while (curr != NULL) {
    next = curr->tail;
    curr->tail = prev;
    prev = curr;
    curr = next;
  }
  *list = prev;

  // while (prev != NULL) {
  //   prev->size = size--; //set and dec.
  //   prev = prev->tail;
  // }
}