#ifndef EXPR_H
#define EXPR_H

#include <stdlib.h>
#include "value.h"
#include "list.h"
#include "scope.h"

typedef enum {
  ValExpr,
  IdentExpr,
  ApplyExpr,
  TypeExpr,
  ListExpr,
  DeclExpr,
  FunExpr,
  BlockExpr,
  TypedValExpr,
  ReturnExpr,
  ifExpr,
} ExprType;

typedef struct {
  ExprType etype;
  size_t size;
  void *subexprs;
} Expr;

Expr *getExprSubexpr(Expr *expr, size_t i);
Expr *makeValExpr(ValType type, void *val);
Expr *makeIdentExpr(char *ident);
Expr *makeApplyExpr(Expr *func, Expr *args);
Expr *makeListExpr(List *list);

Expr *makeUnaryExpr(ExprType etype, Expr *arg);
Expr *makeBinExpr(ExprType etype, Expr *arg1, Expr *arg2);
Expr *makeTernaryExpr(ExprType etype, Expr *arg1, Expr *arg2, Expr *arg3);

Expr *makeBlockExpr(Scope *scope, Expr *listExpr);

char *getIdentExprIdent(Expr *expr);

Expr *makeDeclExpr(Expr *typeExpr, Expr *identExpr);
Expr *makeFunExpr(Expr *type, Expr *body);
Expr *makeTypeExpr(Expr *type);
Expr *makeTypedValExpr(Expr *type, Expr *ident);
Expr *makeReturnExpr(Expr *expr);
Expr *makeNativeExpr(Expr *func, List *args);
Expr *makeIfExpr(Expr *cond, List *then_else);

Expr *makeExprArgs(ExprType type, List *args);


void freeExprList(List *list);
void freeExpr(void *ptr);


#endif