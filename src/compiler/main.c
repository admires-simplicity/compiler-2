#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>
#include <stdarg.h>

#include "../common/trie.h"
#include "../common/list.h"
#include "../common/value.h"
#include "../common/expr.h"
#include "../common/scope.h"
#include "../common/parser.h"


void evalFun(Expr *funExpr, Scope *scope);
int evalExpr(Expr *expr, Scope *scope);


bool reserved(char *ident) {
  //change this to checking if ident is in trie
  return strcmp(ident, "return") == 0;
}


Trie *gloabalIdentifiers;
void initializeGloabalIdentifiers() {
  // this will cause a leak if called more than once (without freeing)
  gloabalIdentifiers = makeTrie();
  trieAdd(gloabalIdentifiers, "printf", makeValExpr(ValExpr, NULL));
  trieAdd(gloabalIdentifiers, "scanf", makeValExpr(ValExpr, NULL));
  trieAdd(gloabalIdentifiers, "add", makeValExpr(ValExpr, NULL));
}



void printValExpr(Expr *expr) {
  // //switch ((ValType)expr->subexprs) {
  // // switch (((Expr *)expr->subexprs)->etype) {
  // switch ((Val *((Expr *)expr->subexprs)->vtype) {
  //   case IntVal:
  //     printf("%d\n", *((int *)expr->subexprs));
  //     break;
  //   case BoolVal:
  //     printf("%s\n", *((int *)expr->subexprs) ? "#t" : "#f");
  //     break;
  //   case StringVal:
  //     printf("\"%s\"\n", (char *)expr->subexprs);
  //     break;
  //   default:
  //     printf("Unknown or unimplemented val type %d\n", (long int)expr->subexprs);
  //     break;
  // }
  switch (((Val *)(expr->subexprs))->vtype) {
    case IntVal:
      printf("%d", *(int *)(((Val *)(expr->subexprs))->val));
      break;
    case BoolVal:
      printf("%s", *(int *)(((Val *)(expr->subexprs))->val) ? "#t" : "#f");
      break;
    case StringVal:
      printf("\"%s\"", (char *)(((Val *)(expr->subexprs))->val));
      break;
    default:
      printf("Unknown or unimplemented val type %d\n", ((Val *)(expr->subexprs))->vtype);
      break;
  }
}

void printIdentExpr(Expr *expr) {
  printf("%s", (char *)expr->subexprs);
}

void printExpr(Expr *expr) {
  //static int c = 0;
  //printf("[expr %d etype %d]: ", ++c, expr->etype);
  if (expr == NULL) {
    printf("NULL\n");
    return;
  }
  switch (expr->etype) {
    case ValExpr:
      printValExpr(expr);
      break;
    case IdentExpr:
      printIdentExpr(expr);
      break;
    case ApplyExpr:
      static int level = 0;
      level++;
      Expr *func = getExprSubexpr(expr, 0);
      Expr *args = getExprSubexpr(expr, 1);
      List *argList = args->subexprs;
      printf("(");
      printExpr(func);
      //printf(" [expr %d etype %d]: ", ++c, args->etype);
      while (argList != NULL) {
        printf(" ");
        printExpr(argList->head);
        argList = argList->tail;
      }
      --level;
      printf(")");
      if (level == 0) printf("\n");
      break;
    default:
      printf("(%s): Unknown or unimplemented expr type %d\n", __func__, expr->etype);
      break;
  }
}

void map(List *list, void (*f)(Expr *)) {
  while (list != NULL) {
    f(list->head);
    list = list->tail;
  }
}

void println(char *str) {
  printf("%s\n", str);
}




Expr *trySpecialForm(char *ident, char *expectIdent, size_t expectArgs,
  List *args, ExprType etype /*, Expr *(*makeExpr)(Expr *, Expr *)*/) {
  
  if (strcmp(ident, expectIdent) == 0) {
    if (listSize(args) != expectArgs) {
      printf("Error: %s needs %d arguments. Given %d\n", expectIdent, expectArgs, listSize(args));
      exit(1);
    }
    //return makeExpr(args->head, args->tail->head);
    return makeExprArgs(etype, args);
  }
  else return NULL;
}

Expr *parseApplyExpr(char *source, size_t *ip) {
  size_t i = *ip;

  i = skipWhitespace(source, i);

  if (source == NULL) {
    printf("Error: source is NULL\n");
    exit(1);
  }

  if (source[i] == '\0') {
    *ip = i;
    return NULL;
  }

  if (source[i] != '(') {
    printf("Expected '(', got '%c'\n", source[i]);
    exit(1);
  }

  i = skipWhitespace(source, i + 1);

  if (source[i] == ')') {
    *ip = i + 1;
    return makeListExpr(NULL); // do I want this?
  }

  Expr *func = parseExpr(source, &i); // this could just be inside the loop. Should it?
  Expr *arg = NULL;
  List *args = NULL;
  while (source[i] != ')') {
    arg = parseExpr(source, &i);
    args = makeList(arg, args);
    i = skipWhitespace(source, i);
  }
  reverseList(&args);
  *ip = i + 1;

  if (func->etype == IdentExpr) {
    char *ident = getIdentExprIdent(func);
    
    // if (strcmp(ident, "decl") == 0) {
    //   //if (args->size != 2) {
    //   if (listSize(args) != 2) {
    //     //printf("Error: decl needs 2 arguments. Given %d\n", args->size); // line number and position would be nice
    //     printf("Error: decl needs 2 arguments. Given %d\n", listSize(args)); // line number and position would be nice
    //     exit(1); // TODO: better error handling.
    //   }
    //   return makeDeclExpr(args->head, args->tail->head);
    // }

    // if (strcmp(ident, "fun") == 0) {
    //   //if (args->size != 2) {
    //   if (listSize(args) != 2) {
    //     //printf("Error: fun needs 2 arguments. Given %d\n", args->size);
    //     printf("Error: fun needs 2 arguments. Given %d\n", listSize(args));
    //     exit(1); // TODO: better error handling.
    //   }
    //   return makeFunExpr(args->head, args->tail->head);
    // }

    Expr *specialExpr = NULL;
    // if ((specialExpr = trySpecialForm(ident, "decl", 2, args, &makeDeclExpr)) != NULL ||
    //     (specialExpr = trySpecialForm(ident, "fun", 2, args, &makeFunExpr)) != NULL ||
    //     (specialExpr = trySpecialForm(ident, "if", 3, args, &makeIfExpr)) != NULL) { // this is just broken until I refactor to makeNExpr TODO: fix
    //   return specialExpr;
    // }
    if ((specialExpr = trySpecialForm(ident, "decl", 2, args, DeclExpr)) != NULL ||
        (specialExpr = trySpecialForm(ident, "fun", 2, args, FunExpr)) != NULL ||
        (specialExpr = trySpecialForm(ident, "if", 3, args, ifExpr)) != NULL) {
      return specialExpr;
    }

    //TODO: add DefExpr
    //TODO: do I need to handle lambdas here??
  }
  
  //move preceding into function
  
  if (func->etype == IdentExpr && reserved(getIdentExprIdent(func))) {
    return makeNativeExpr(func, args);
  }
  return makeApplyExpr(func, makeListExpr(args));
}

Expr *parseExpr(char *source, size_t *ip) {
  size_t i = *ip;

  if (source == NULL) {
    printf("Error: source is NULL\n");
    exit(1);
  }

  i = skipWhitespace(source, i); // not sure if I want this to be handled here or before calling this function
  
  switch (source[i]) {
    case '\0':
      *ip = i;
      return NULL;
    case '(':
      Expr *applyExpr = parseApplyExpr(source, &i);
      *ip = i; // maybe I should just dereference ip directly?
      return applyExpr;
    default:
      Expr *valExpr = parseValue(source, &i);
      *ip = i;
      return valExpr;
  }
}

Scope *makeScope(Scope *parent, Trie *identifiers) { // SHOULD I PASS TRIE? LIST? NOTHING?
  Scope *scope = malloc(sizeof(Scope));
  scope->identifiers = identifiers;
  scope->parent = parent;
  return scope;
}

Scope *makeGlobalScope() {
  initializeGloabalIdentifiers(); // SHOULD THIS BE HERE?
  return makeScope(NULL, gloabalIdentifiers);
}

Scope *makeLocalScope(Scope *parent) {
  return makeScope(parent, makeTrie());
}


char *getIdentExprIdent(Expr *expr) {
  return (char *)expr->subexprs;
}

void emitExpr(Expr *expr, Scope *scope) {
  static int level = 0;
  //printf("emit expr %d\n", expr->etype);
  switch (expr->etype) {
    case TypeExpr:
      printf("%s", getIdentExprIdent(getExprSubexpr(expr, 0)));
      break;
    case ValExpr:
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
      evalExpr(getExprSubexpr(expr, 0), scope);
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


// bool isDecl(Expr *expr) {
//   return expr->etype == DeclExpr || expr->etype == FunExpr; // later add DefExpr
// }

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
      forwardReturn(consequent);
      forwardReturn(alternate);
      ((Expr **)(expr->subexprs))[1] = forwardReturn(consequent);
      ((Expr **)(expr->subexprs))[2] = forwardReturn(alternate);
      return expr;
      break;
    case ApplyExpr:
    case ValExpr:
    case IdentExpr:
      // Expr *returnExpr = makeReturnExpr(expr);
      // *expr = *returnExpr;    
      return makeReturnExpr(expr);
      break;
    default:
      printf("(%s): Unknown or unimplemented expr type %d\n", __func__, expr->etype);
      break;
  }
  return NULL;
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
    case TypeExpr: //fallthrough
    case ValExpr:
    case IdentExpr:
    case ApplyExpr:
    case ListExpr:
    case TypedValExpr: // temporary?
    case DeclExpr:
      emitExpr(expr, scope);
      break;
    case ReturnExpr:
      evalReturn(expr, scope);
      break;
    default:
      printf("(%s): Unknown or unimplemented expr type %d\n", __func__, expr->etype);
      break;
  }
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


int main(char argc, char **argv) {
  if (argc != 2) {
    printf("Usage: %s <filename>\n", argv[0]);
    exit(1);
  }

  List *program = NULL;

  char *source = readSource(argv[1]);

  size_t si = 0;
  while (source[si] != '\0') {
    //Expr *expr = parseValue(source, &si); 
    si = skipWhitespace(source, si);
    //Expr *expr = parseList(source, &si);
    Expr *expr = parseExpr(source, &si);
    if (expr != NULL) program = makeList(expr, program);
  }

  //printf("program size: %d\n", listSize(program));
  compile(&program);
  //printf("program size: %d\n", listSize(program));

  //printList(program);

  freeExprList(program);

  free(source);
}

// instead of "evalProgram" and "evalFun" I should just have "evalFun" and create a main fun in main.

// TODO: add native functions +, -, *, /

// TODO: refactor into modules

// TODO: start adding tests

// TODO: finish implementing ifExpr -- implement evalFun ifExpr case 

// TODO: add type checking

// TODO: fix memory leaks

// TODO: fix block functions so ex7 and ex8 work

// TODO: add indentation to outputted code for readability

// TODO: check evalReturn to make sure arbitrarily nested ifs and blocks word correctly

// TODO: check if nested function definitions are working (I don't think they
//       should be) and fix them if not