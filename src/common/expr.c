#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>

#include "expr.h"
#include "value.h"
#include "list.h"
#include "scope.h"

size_t exprArity[] = {
  [ValExpr] = 1,
  [IdentExpr] = 0,
  [ApplyExpr] = 2,
  [TypeExpr] = 1,
  [ListExpr] = 1,
  [DeclExpr] = 2,
  [FunExpr] = 2,
  [BlockExpr] = 2,
  [TypedValExpr] = 2,
  [ReturnExpr] = 1,
  [ifExpr] = 3,

  [PlusExpr] = 2,
  [MinusExpr] = 2,
  [TimesExpr] = 2,
  [DivideExpr] = 2,
  [ModExpr] = 2,
  [LessExpr] = 2,
  [LessEqExpr] = 2,
  [GreaterExpr] = 2,
  [GreaterEqExpr] = 2,
  [EqExpr] = 2,
  [AndExpr] = 2,
  [OrExpr] = 2,
  [XorExpr] = 2,
  [NotExpr] = 1,
  [NegExpr] = 1,

};

// maybe this should be void **getSubexpr ?
Expr *getExprSubexpr(Expr *expr, size_t i) {
  return ((Expr **)(expr->subexprs))[i];
}

Expr *makeValExpr(ValType type, void *val) {
  Expr *expr = malloc(sizeof(Expr));
  expr->etype = ValExpr;
  expr->size = 1;
  expr->subexprs = makeVal(type, val);
  return expr;
}

Expr *makeIdentExpr(char *ident) {
  Expr *expr = malloc(sizeof(Expr));
  expr->etype = IdentExpr;
  expr->size = 1;
  expr->subexprs = ident;
  return expr;
}

Expr *makeApplyExpr(Expr *func, Expr *args) {
  assert(args->etype == ListExpr);
  Expr *expr = malloc(sizeof(Expr));
  expr->etype = ApplyExpr;
  expr->size = 2;
  expr->subexprs = malloc(expr->size);
  ((Expr **)expr->subexprs)[0] = func;
  ((Expr **)expr->subexprs)[1] = args;
  return expr;
}

Expr *makeListExpr(List *list) {
  Expr *expr = malloc(sizeof(Expr));
  expr->etype = ListExpr;
  expr->subexprs = list;
  return expr;
}

Expr *makeUnaryExpr(ExprType etype, Expr *arg) {
  Expr *expr = malloc(sizeof(Expr));
  expr->etype = etype;
  expr->size = 1;
  expr->subexprs = malloc(sizeof(Expr *));
  ((Expr **)expr->subexprs)[0] = arg;
  return expr;
}

Expr *makeBinExpr(ExprType etype, Expr *arg1, Expr *arg2) {
  Expr *expr = malloc(sizeof(Expr));
  expr->etype = etype;
  expr->size = 2;
  expr->subexprs = malloc(2 * sizeof(Expr *));
  ((Expr **)expr->subexprs)[0] = arg1;
  ((Expr **)expr->subexprs)[1] = arg2;
  return expr;
}

Expr *makeTernaryExpr(ExprType etype, Expr *arg1, Expr *arg2, Expr *arg3) {
  Expr *expr = malloc(sizeof(Expr));
  expr->etype = etype;
  expr->size = 3;
  expr->subexprs = malloc(3 * sizeof(Expr *));
  ((Expr **)expr->subexprs)[0] = arg1;
  ((Expr **)expr->subexprs)[1] = arg2;
  ((Expr **)expr->subexprs)[2] = arg3;
  return expr;
}

//I could abstract these to makeNExpr with int and List* arguments.
//I think it would require too much refactoring right now.

// Expr *makeNExpr(ExprType etype, int n, List *args) {
//   assert(listSize(args) == n);
//   Expr *expr = malloc(sizeof(Expr));
//   expr->etype = etype;
//   expr->size = n;
//   expr->subexprs = malloc(n * sizeof(Expr *));
//   List *curr = args;
//   for (int i = 0; i < n; ++i) {
//     ((Expr **)expr->subexprs)[i] = curr->head;
//     curr = curr->tail;
//   }
// }

Expr *makeBlockExpr(Scope *scope, Expr *listExpr) {
  assert(listExpr->etype == ListExpr);
  Expr *expr = malloc(sizeof(Expr));
  expr->etype = BlockExpr;
  expr->size = 2;
  expr->subexprs = malloc(sizeof(Scope *) + sizeof(Expr *));
  ((Scope **)expr->subexprs)[0] = scope; // hack?
  ((Expr **)expr->subexprs)[1] = listExpr;
  return expr;
}

Expr *makeDeclExpr(Expr *typeExpr, Expr *identExpr) {
  //assert(typeExpr->etype == TypeExpr); // TODO
  assert(identExpr->etype == IdentExpr);
  return makeBinExpr(DeclExpr, typeExpr, identExpr);
}

Expr *makeFunExpr(Expr *type, Expr *body) {
  //assert(type->etype == TypeExpr); // TODO
  //assert(body->etype == BlockExpr || body->etype == ApplyExpr);
  return makeBinExpr(FunExpr, type, body);
}

Expr *makeTypeExpr(Expr *type) {
  assert(type->etype == IdentExpr);
  return makeUnaryExpr(TypeExpr, type);
}

Expr *makeTypedValExpr(Expr *type, Expr *ident) {
  assert(type->etype == IdentExpr); // TODO: make better. add actual type checking.
  //assert(ident->etype == IdentExpr);
  assert(ident->etype == ListExpr);
  assert(((Expr *)(((List *)ident->subexprs)->head))->etype == IdentExpr);
  return makeBinExpr(TypedValExpr, type, ((List *)ident->subexprs)->head);
}

Expr *makeReturnExpr(Expr *expr) {
  return makeUnaryExpr(ReturnExpr, expr);
}

Expr *makeNativeExpr(Expr *func, List *args) {
  char *ident = getIdentExprIdent(func);
  if (strcmp(ident, "return") == 0) {
    assert(listSize(args) == 1);
    return makeReturnExpr(args->head);
  }
}

Expr *makeIfExpr(Expr *cond, List *then_else) {
  assert(listSize(then_else) == 2);
  return makeTernaryExpr(ifExpr, cond, then_else->head, then_else->tail->head);
}

Expr *makeExprArgs(ExprType type, List *args) {
  size_t size = listSize(args);
  Expr *expr = malloc(sizeof(Expr));
  expr->etype = type;
  expr->size = size;
  expr->subexprs = malloc(size * sizeof(Expr *));
  List *curr = args;
  for (int i = 0; i < size; ++i) {
    ((Expr **)expr->subexprs)[i] = curr->head;
    curr = curr->tail;
  }
  return expr;
  // somehow I should do parity checking on etype and size...
}

char *getIdentExprIdent(Expr *expr) {
  return (char *)expr->subexprs;
}

void freeExprList(List *list) {
  if (list == NULL) {
    printf("Warning (compiler bug): tried to free NULL list. This shouldn't happen.\n");
    return;
  }
  if (list->tail != NULL) {
    freeExprList(list->tail);
  }
  freeExpr(list->head);
  free(list);
}

/*
 * must be passed Expr ptr
 */
void freeExpr(void *ptr) {
  Expr *expr = (Expr *)ptr;
  switch (expr->etype) {
    case ValExpr:
      freeValue(expr->subexprs);
      break;
    case IdentExpr:
      free(expr->subexprs);
      break;
    case ListExpr:
      //freeList(expr->subexprs);
      freeListWith(expr->subexprs, &freeExpr);
      break;
    case BlockExpr:
      freeScope((Scope *)getExprSubexpr(expr, 0)); // hacky -- maybe wrap getExprSubexpr in getScopeExprScope (?) (would also have to change Scope to ScopeExpr)
      freeExpr(getExprSubexpr(expr, 1));
      free(expr->subexprs);
      break;
    case ApplyExpr: // fallthrough
    case DeclExpr: 
    case FunExpr:
    case TypedValExpr:
      freeExpr(getExprSubexpr(expr, 0));
      freeExpr(getExprSubexpr(expr, 1));
      free(expr->subexprs);
      break;
    case TypeExpr:
    case ReturnExpr:
      freeExpr(getExprSubexpr(expr, 0));
      free(expr->subexprs);
      break;
    default:
      printf("(%s): Unknown or unimplemented expr type %d\n", __func__, expr->etype);
      break;
  }
  free(expr);
}

/* Maybe poorly named.
 * Check if expr is a returnable expression.
 * maybe "hasValue" is better?
 * but "hasValue" should work for basically all expressions
 * in particular it would be cool if it worked for TypeExpr
 */
bool arithmeticExpr(Expr *expr) {
  switch (expr->etype) {
    case ValExpr:
    case IdentExpr:
    case ApplyExpr:
    case PlusExpr:
    case MinusExpr:
    case TimesExpr:
    case DivideExpr:
    case ModExpr:
    case LessExpr:
    case LessEqExpr:
    case GreaterExpr:
    case GreaterEqExpr:
    case EqExpr:
    case AndExpr:
    case OrExpr:
    case XorExpr:
    case NotExpr:
    case NegExpr:
      return true;
    default:
      return false;
  }
}