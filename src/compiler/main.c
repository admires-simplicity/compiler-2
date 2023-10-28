#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "../common/trie.h"

typedef enum {
  ValExpr,
  IdentExpr,
  ApplyExpr,
  TypeExpr,
  ListExpr,
} ExprType;

typedef enum {
  IntVal,
  BoolVal,
  StringVal,
} ValType;

typedef struct {
  ExprType type;
  size_t size;
  void *subexprs;
} Expr;

typedef struct List List;
struct List {
  size_t size;
  void *head;
  List *tail;
};

List *makeList(void *head, List *tail) {
  List *list = malloc(sizeof(List));
  list->size = tail == NULL ? 1 : tail->size + 1;
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

/* Prints a list. Only works for char* list.
 */
void print_list(List *list) {
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

Expr *makeValExpr(ValType type, void *val) {
  Expr *expr = malloc(sizeof(Expr));
  expr->type = ValExpr;
  expr->size = 1;
  expr->subexprs = val;
  return expr;
}

Expr *makeIdentExpr(char *ident) {
  Expr *expr = malloc(sizeof(Expr));
  expr->type = IdentExpr;
  expr->size = 1;
  expr->subexprs = ident;
  return expr;
}

Expr *makeApplyExpr(Expr *func, Expr *args) {
  assert(args->type == ListExpr);
  Expr *expr = malloc(sizeof(Expr));
  expr->type = ApplyExpr;
  expr->size = sizeof(Expr *) * 2;
  expr->subexprs = malloc(expr->size);
  ((Expr **)expr->subexprs)[0] = func;
  ((Expr **)expr->subexprs)[1] = args;
  return expr;
}

Expr *makeListExpr(List *list) {
  Expr *expr = malloc(sizeof(Expr));
  expr->type = ListExpr;
  expr->size = list->size;
  // List **list_ptr = malloc(sizeof(List *));
  // *list_ptr = list;
  // expr->subexprs = list_ptr;
  expr->subexprs = list;
  return expr;
}

Trie *gloabal_identifiers;
void initialize_gloabal_identifiers() {
  // this will cause a leak if called more than once (without freeing)
  gloabal_identifiers = makeTrie();
  trieAdd(gloabal_identifiers, "printf", makeValExpr(ValExpr, NULL));
  trieAdd(gloabal_identifiers, "scanf", makeValExpr(ValExpr, NULL));
  trieAdd(gloabal_identifiers, "add", makeValExpr(ValExpr, NULL));
}

int main() {
  char *a = "hello";
  char *b = "world";
  int *c = malloc(sizeof(int));
  *c = 42;

  List *list = malloc(3 * sizeof(List));
  list[0].head = a;
  list[0].tail = &list[1];
  list[1].head = b;
  list[1].tail = &list[2];
  list[2].head = c;
  list[2].tail = NULL;


  List *l = makeList("1", makeList("2", makeList("3", NULL)));
  print_list(l);
  freeList(l);

  List *l2;
  
  
}