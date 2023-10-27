#include <stdio.h>
#include <stdlib.h>

#include "../common/trie.h"

typedef enum {
  ValExpr,
  ApplyExpr,
} ExprType;

typedef enum {
  IntVal,
  BoolVal,
} ValType;

typedef struct {
  ExprType type;
  size_t size;
  void *subexprs;
} Expr;

typedef struct List List;
struct List {
  void *head;
  List *tail;
};

Expr *makeValExpr(ValType type, void *val) {
  Expr *expr = malloc(sizeof(Expr));
  expr->type = ValExpr;
  expr->size = sizeof(ValType);
  expr->subexprs = val;
  return expr;
}

Expr *makeApplyExpr(Expr *func, Expr *arg) {
  Expr *expr = malloc(sizeof(Expr));
  expr->type = ApplyExpr;
  expr->size = sizeof(Expr *) * 2;
  expr->subexprs = malloc(expr->size);
  ((Expr **)expr->subexprs)[0] = func;
  ((Expr **)expr->subexprs)[1] = arg;
  return expr;
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

}