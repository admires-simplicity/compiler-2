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
  //List *decls = gatherFrom(subexprList, &isDecl);
  List *funs = gatherFrom(subexprList, &isFun);

  // printf("decls size: %d\n", decls->size);
  // printf("subexprList size: %d\n", subexprList->size);
  //printf("(%s): decls size: %d\n", __func__, listSize(decls));
  //printf("(%s): subexprList size: %d\n", __func__, listSize(subexprList));

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
      //printf("(%s): Unknown or unimplemented expr type %d\n", __func__, returnExpr->etype);
      printf("(%s): Bad semantics -- unexpected expr type %d\n", __func__, returnExpr->etype);
      break;
  }
  return 0;
}

int evalExpr(Expr *expr, Scope *scope) {
  switch (expr->etype) {
    // case TypeExpr: //fallthrough
    // case ValExpr:
    // case IdentExpr:
    // case ApplyExpr:
    // case ListExpr:
    // case TypedValExpr: // temporary?
    // case DeclExpr:
    //   emitExpr(expr, scope);
    //   break;
    // case ReturnExpr:
    //   evalReturn(expr, scope);
    //   break;
    // default:
    //   printf("(%s): Unknown or unimplemented expr type %d\n", __func__, expr->etype);
    //   break;
    case ReturnExpr:
      evalReturn(expr, scope);
      break;
    default:
      emitExpr(expr, scope);
      break;
  }
  return 0;
}

// int evalDeclList(List *declList, Scope *scope) {

// }

List *evalTypes(Expr *typeListExpr) {
  return NULL;
}

Expr *interpretType(Expr *typeExpr) {
  switch (typeExpr->etype) {
    case IdentExpr:
      //return makeTypedValExpr(typeExpr, NULL); // Type, NULL (type without identifier)
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
    //args = makeList(makeTypedValExpr(getExprSubexpr(curr->head, 0), getExprSubexpr(curr->head, 1)), args);
    curr = curr->tail;
  }
  //returnType = curr->head;
  returnType = interpretType(curr->head);
  reverseList(&args);

  evalExpr(returnType, scope); // I don't think it really matters whether I pass scope or localScope here, except if I maybe want to allow type shadowing... which seems bad.
  printf(" %s(", ident);
  //map(args, &evalExpr); // maybe should be "evalTypedValExpr" or something
  // for (List *curr = args; curr != NULL; curr = curr->tail) {
  //   evalExpr(curr->head, scope);
  //   if (curr->tail != NULL) printf(", ");
  // }
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
  //List *decls = gatherFrom(instructionList, &isDecl);

  for (List *curr = funs; curr != NULL; curr = curr->tail) {
    evalFun(curr->head, scope);
  }

  printf("int main() {\n");
  //evalInstructionList(instructionList, scope);
  evalBlockExpr(makeBlockExpr(scope, instructions));
  printf("}\n");

  // printf("(%s): decls size: %d\n", __func__, decls->size);
  // printf("(%s): instructionList size: %d\n", __func__, instructionList->size);
  // printf("(%s): decls size: %d\n", __func__, listSize(decls));
  // printf("(%s): instructionList size: %d\n", __func__, listSize(instructionList));

  return 0;
}

int compile(List **program) {
  Scope *globalScope = makeGlobalScope();
  reverseList(program);

  Expr *programBlock = makeBlockExpr(globalScope, makeListExpr(*program));

  //printf("int main() {\n");
  
  // List *curr = *program;
  // while (curr != NULL) {
  //   Expr *expr = curr->head;
  //   emitExpr(expr, globalScope);
  //   curr = curr->tail;
  // }
  
  //printf("}\n");
  
  evalProgram(programBlock);
  
  // Expr *main = makeFunExpr(makeApplyExpr(makeIdentExpr("main"), makeListExpr(makeList(makeIdentExpr("int"), NULL))), programBlock);
  // evalFun(main, globalScope);

  freeScope(globalScope);
  return 0;
}
