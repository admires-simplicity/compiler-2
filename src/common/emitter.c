#include <stdio.h>
#include <stdbool.h>

#include "emitter.h"
#include "expr.h"
#include "scope.h"

#include "compiler.h" // this feels bad?  TODO: remove?

#include "printer.h"

bool unaryEmitType(Expr *expr) {
  switch (expr->etype) {
    case NotExpr:
    case NegExpr:
      return true;
    default:
      return false;
  }
}

bool binEmitType(Expr *expr) {
  switch (expr->etype) {
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
      return true;
    default:
      return false;
  }
}

void emitUnOp(Expr *expr) {
  switch (expr->etype) {
    case NotExpr:
      printf("!");
      break;
    case NegExpr:
      printf("-");
      break;
    default:
      printf("(%s): Unknown or unimplemented expr type %d\n", __func__, expr->etype);
      break;
  }
}

void emitBinOp(Expr *expr) {
  switch (expr->etype) {
    case PlusExpr:
      printf("+");
      break;
    case MinusExpr:
      printf("-");
      break;
    case TimesExpr:
      printf("*");
      break;
    case DivideExpr:
      printf("/");
      break;
    case ModExpr:
      printf("%%");
      break;
    case LessExpr:
      printf("<");
      break;
    case LessEqExpr:
      printf("<=");
      break;
    case GreaterExpr:
      printf(">");
      break;
    case GreaterEqExpr:
      printf(">=");
      break;
    case EqExpr:
      printf("==");
      break;
    case AndExpr:
      printf("&&");
      break;
    case OrExpr:
      printf("||");
      break;
    case XorExpr:
      printf("^");
      break;
    default:
      printf("(%s): Unknown or unimplemented expr type %d\n", __func__, expr->etype);
      break;
  }
}

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
      if (unaryEmitType(expr)) {
        emitUnOp(expr);
        emitExpr(getExprSubexpr(expr, 0), scope);
        break;
      }
      else if (binEmitType(expr)) {
        emitExpr(getExprSubexpr(expr, 0), scope);
        emitBinOp(expr);
        emitExpr(getExprSubexpr(expr, 1), scope);
        break;
      }
      printf("(%s): Unknown or unimplemented expr type %d\n", __func__, expr->etype);
      break;
  }
}
