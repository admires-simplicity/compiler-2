#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

#include "expr.h"
#include "scope.h"
#include "compiler.h"
#include "emitter.h"

#include "list.h"
#include "utility.h"

void evalFun(Expr *funExpr, Scope *scope);
int evalExpr(Expr *expr, Scope *scope);

bool isFun(Expr *expr) {
  return expr->etype == FunExpr;
}
int evalBlockExpr(Expr *expr) {
  assert(expr->etype == BlockExpr || expr->etype == ApplyExpr);
  Scope *scope = (Scope *)getExprSubexpr(expr, 0);
  Expr *listExpr = getExprSubexpr(expr, 1);
  List *subexprList = listExpr->subexprs;
  List *funs = gatherFrom(subexprList, &isFun);

  for (List *curr = funs; curr != NULL; curr = curr->tail) {
    evalFun(curr->head, scope);
    curr = curr->tail;
  }

  List *curr;
  for (curr = subexprList; curr->tail != NULL; curr = curr->tail) {
    evalExpr(curr->head, scope);
  }

  if (((Expr *)curr->head)->etype == ReturnExpr) {
    evalExpr(curr->head, scope);
  } else {
    evalExpr(makeReturnExpr(curr->head), scope);
  }

  return 0;
}

Expr *forwardReturn(Expr *expr) {
  switch (expr->etype) {
    case BlockExpr:
      Expr *blockList = getExprSubexpr(expr, 1);
      List *subexprList = blockList->subexprs;
      List *curr = subexprList;
      assert(curr != NULL);
      while (curr ->tail != NULL) {
        curr = curr->tail;
      }
      curr->head = forwardReturn(curr->head);
      return expr;
      break;
    case ifExpr:
      Expr *consequent = getExprSubexpr(expr, 1);
      Expr *alternate = getExprSubexpr(expr, 2);
      ((Expr **)(expr->subexprs))[1] = forwardReturn(consequent);
      ((Expr **)(expr->subexprs))[2] = forwardReturn(alternate);
      return expr;
      break;
    default:
      if (arithmeticExpr(expr)) {
        return makeReturnExpr(expr);
      }
      printf("(%s): Unknown or unimplemented expr type %d\n", __func__, expr->etype);
      return NULL;
  }
}

int evalReturn(Expr *expr, Scope *scope) {
  assert(expr->etype == ReturnExpr);
  Expr *returnExpr = getExprSubexpr(expr, 0);
  switch (returnExpr->etype) {
    case ifExpr:
    case BlockExpr:
      forwardReturn(returnExpr);
      emitExpr(returnExpr, scope);
      break;
    case ReturnExpr:
      if (returnExpr->etype == ReturnExpr) {
        return evalReturn(returnExpr, scope);
      }
      //else fallthrough
    case ApplyExpr:
    case ValExpr:
    case IdentExpr:
      emitExpr(expr, scope);
      break;
    default:
      printf("(%s): Bad semantics -- unexpected expr type %d\n", __func__, returnExpr->etype);
      break;
  }
  return 0;
}

int evalExpr(Expr *expr, Scope *scope) {
  switch (expr->etype) {
    case ReturnExpr:
      evalReturn(expr, scope);
      break;
    default:
      emitExpr(expr, scope);
      break;
  }
  return 0;
}

List *evalTypes(Expr *typeListExpr) {
  return NULL;
}

Expr *interpretType(Expr *typeExpr) {
  switch (typeExpr->etype) {
    case IdentExpr:
      return makeTypeExpr(typeExpr);
    case ApplyExpr:
      return makeTypedValExpr(getExprSubexpr(typeExpr, 0), getExprSubexpr(typeExpr, 1)); // Type, Name
    default:
      printf("(%s): Unknown or unimplemented expr type %d\n", __func__, typeExpr->etype);
      exit(1);
  }
}

void evalFun(Expr *funExpr, Scope *scope) {
  //should I also declare every function before defining it ???
  assert(funExpr->etype == FunExpr);

  Expr *typeDefinition = getExprSubexpr(funExpr, 0);
  Expr *body = getExprSubexpr(funExpr, 1);
  
  char *ident = getIdentExprIdent(getExprSubexpr(typeDefinition, 0));
  //List *types = evalTypes(getExprSubexpr(typeDefinition, 1)); // TODO : add scope
  List *types = getExprSubexpr(typeDefinition, 1)->subexprs;


  List *args = NULL;
  Expr *returnType = NULL;
  Scope *localScope = makeLocalScope(scope);
  List *curr = types;

  if (curr == NULL) {
    printf("(%s): implicit return type not yet implemented\n", __func__);
    exit(1);
  }

  while (curr->tail != NULL) {
    if (strcmp(getIdentExprIdent(curr->head), "->") == 0) {
      curr = curr->tail;
      continue; // hack. TODO: fix
    }
    // we have to deduce types and declare typed values in the scope of the function for each argument
    
    args = makeList(interpretType(curr->head), args);
    curr = curr->tail;
  }
  returnType = interpretType(curr->head);
  reverseList(&args);

  evalExpr(returnType, scope); // I don't think it really matters whether I pass
  // scope or localScope here, except if I maybe want to allow type shadowing...
  // which seems bad.
  printf(" %s(", ident);
  Expr *argListExpr = makeListExpr(args);
  evalExpr(argListExpr, scope);
  printf(") {\n");
  Expr *blockExpr;
  switch (body->etype) {
    case BlockExpr:
      blockExpr = body;
      break;
    case ApplyExpr:
    case ifExpr:
    case ValExpr:
    case IdentExpr:
      blockExpr = makeBlockExpr(localScope, makeListExpr(makeList(body, NULL)));
      break;
    default:
      printf("(%s): Unknown or unimplemented expr type %d\n", __func__, body->etype);
      exit(1);
  }
  evalBlockExpr(blockExpr);
  printf("}\n");

}

void evalInstructionList(List *instructionList, Scope *scope) {
  printf("%s\n", __func__);
}


int evalProgram(Expr *program) {
  assert(program->etype == BlockExpr);
  Scope *scope = (Scope *)getExprSubexpr(program, 0);
  Expr *instructions = getExprSubexpr(program, 1);
  assert(instructions->etype == ListExpr);
  List *instructionList = instructions->subexprs;
  List *funs = gatherFrom(instructionList, &isFun);

  for (List *curr = funs; curr != NULL; curr = curr->tail) {
    evalFun(curr->head, scope);
  }

  printf("int main() {\n");
  evalBlockExpr(makeBlockExpr(scope, instructions));
  printf("}\n");

  return 0;
}

int compile(List **program) {
  Scope *globalScope = makeGlobalScope();
  reverseList(program);

  Expr *programBlock = makeBlockExpr(globalScope, makeListExpr(*program));
  
  evalProgram(programBlock);
  
  freeScope(globalScope);
  return 0;
}
