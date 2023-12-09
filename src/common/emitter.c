#include <stdio.h>

#include "emitter.h"
#include "expr.h"
#include "scope.h"

#include "compiler.h" // this feels bad?  TODO: remove?

#include "printer.h"

void emitExpr(Expr *expr, Scope *scope) {
  static int level = 0;
  //printf("emit expr %d\n", expr->etype);
  switch (expr->etype) {
    case TypeExpr:
      printf("%s", getIdentExprIdent(getExprSubexpr(expr, 0)));
      break;
    case ValExpr:
      // TODO: change printValExpr to emitValExpr and completely rewrite printer.c/h
      printValExpr(expr);
      break;
    case IdentExpr:
      printf("%s", getIdentExprIdent(expr));
      break;
    case ApplyExpr:
      ++level;
      emitExpr(getExprSubexpr(expr, 0), scope); // assuming func is not a lambda (will have to change later)
      printf("(");
      emitExpr(getExprSubexpr(expr, 1), scope);
      printf(")");
      --level;
      if (level == 0) printf(";\n");
      break;
    case ListExpr:
      List *list = expr->subexprs;
      while (list != NULL) {
        emitExpr(list->head, scope);
        list = list->tail;
        if (list != NULL) printf(", ");
      }
      break;
    case TypedValExpr:
      emitExpr(getExprSubexpr(expr, 0), scope);
      printf(" ");
      emitExpr(getExprSubexpr(expr, 1), scope);
      break;
    case ReturnExpr:
      ++level;
      printf("return ");
      evalExpr(getExprSubexpr(expr, 0), scope); // TODO: move this somewhere else??
      --level;
      printf(";\n");
      break;
    case DeclExpr:
      emitExpr(getExprSubexpr(expr, 0), scope);
      printf(" ");
      emitExpr(getExprSubexpr(expr, 1), scope);
      printf(";\n");
      break;
    case ifExpr:
      Expr *cond = getExprSubexpr(expr, 0);
      Expr *consequent = getExprSubexpr(expr, 1);
      Expr *alternate = getExprSubexpr(expr, 2);
      printf("if (");
      ++level;
      emitExpr(cond, scope);
      --level;
      printf(") {\n");
      emitExpr(consequent, scope);
      printf("} else {\n");
      emitExpr(alternate, scope);
      printf("}\n");
      break;
    default:
      printf("(%s): Unknown or unimplemented expr type %d\n", __func__, expr->etype);
      break;
  }
}
