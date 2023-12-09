#include "utility.h"

#include <stdbool.h>

#include "list.h"
#include "expr.h"

void map(List *list, void (*f)(Expr *)) {
  while (list != NULL) {
    f(list->head);
    list = list->tail;
  }
}

List *gatherFrom(List *list, bool (*gather)(Expr *)) {
  List *gathered = NULL;
  List *current = list;

  while (list != NULL) {
    if (gather(list->head)) {
      gathered = makeList(list->head, gathered);
      *list = *(list->tail);
    }
    else list = list->tail;
  }
  reverseList(&gathered);
  return gathered;
}