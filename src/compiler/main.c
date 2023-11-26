#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>
#include <stdarg.h>

#include "../common/trie.h"

typedef enum {
  IntVal,
  BoolVal,
  StringVal,
} ValType;

typedef struct {
  ValType vtype;
  void *val;
} Val;

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

typedef struct List List;
struct List {
  //size_t size;
  void *head;
  List *tail;
};

typedef struct Scope Scope;
struct Scope {
  Trie *identifiers;
  Scope *parent;
};

void freeScope(Scope *scope);
char *getIdentExprIdent(Expr *expr);
void evalFun(Expr *funExpr, Scope *scope);
int evalExpr(Expr *expr, Scope *scope);
int listSize(List *list);




// maybe this should be void **getSubexpr ? (can you do (void **) ?)
Expr *getExprSubexpr(Expr *expr, size_t i) {
  return ((Expr **)(expr->subexprs))[i];
}

List *makeList(void *head, List *tail) {
  List *list = malloc(sizeof(List));
  //list->size = tail == NULL ? 1 : tail->size + 1;
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

void freeListWith(List *list, void (*freeValue)(void *)) {
  if (list->tail != NULL) {
    freeListWith(list->tail, freeValue);
  }
  freeValue(list->head);
  free(list);
}

/* Prints a list. Only works for char* list.
 */
void printList(List *list) {
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

Val *makeVal(ValType type, void *val) {
  Val *v = malloc(sizeof(Val));
  v->vtype = type;
  v->val = val;
  return v;
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

Expr *makeApplyExpr(Expr *func, Expr *args) { // TODO: change to using makeBinExpr
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
  //expr->size = list->size;
  // List **list_ptr = malloc(sizeof(List *));
  // *list_ptr = list;
  // expr->subexprs = list_ptr;
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


Expr *makeBlockExpr(Scope *scope, Expr *listExpr) { // TODO : change to using makeBinExpr
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
  //printf("%d\n", ident->etype);
  //assert(ident->etype == IdentExpr);
  assert(ident->etype == ListExpr);
  assert(((Expr *)(((List *)ident->subexprs)->head))->etype == IdentExpr);
  return makeBinExpr(TypedValExpr, type, ((List *)ident->subexprs)->head);
}

Expr *makeReturnExpr(Expr *expr) {
  return makeUnaryExpr(ReturnExpr, expr);
}

bool reserved(char *ident) {
  //change this to checking if ident is in trie
  return strcmp(ident, "return") == 0;
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

Trie *gloabalIdentifiers;
void initializeGloabalIdentifiers() {
  // this will cause a leak if called more than once (without freeing)
  gloabalIdentifiers = makeTrie();
  trieAdd(gloabalIdentifiers, "printf", makeValExpr(ValExpr, NULL));
  trieAdd(gloabalIdentifiers, "scanf", makeValExpr(ValExpr, NULL));
  trieAdd(gloabalIdentifiers, "add", makeValExpr(ValExpr, NULL));
}

size_t skipComment(char *source, size_t i) {
  while (source[i] != '\n') {
    ++i;
  }
  return i;
}

size_t skipWhitespace(char *source, size_t i) { // maybe I should change this to size_t *i
  while (source[i] == ' ' || source[i] == '\n' || source[i] == '\t') {
    ++i;
  }
  if (source[i] == ';') {
    //return skipWhitespace(source, skipComment(source, i));
    while (source[i] != '\n') {
      ++i;
    }
    return skipWhitespace(source, i);
  }
  return i;
}

bool identChar(char c) {
  switch (c) {
    case ' ':
    case '\n':
    case '\t':
    case '(':
    case ')':
    case ';':
    case '\0':
      return false;
    default:
      return true;
  }
}

Expr *parseStringValue(char *source, size_t *ip) {
  size_t i = *ip;
  size_t j = i + 1;
  while (source[j] != '"') {
    j++;
  }
  char *val = malloc((j - i + 1) * sizeof(char));
  strncpy(val, source + i + 1, j - i - 1);
  val[j - i - 1] = '\0';
  *ip = j + 1;
  return makeValExpr(StringVal, val);
}

Expr *parseBool(char *source, size_t *ip) {
  size_t i = *ip;
  if (source[i + 1] == 't') {
    *ip = i + 2;
    return makeValExpr(BoolVal, (void *)1);
  }
  if (source[i + 1] == 'f') {
    *ip = i + 2;
    return makeValExpr(BoolVal, (void *)0);
  } 
  else {
    printf("Malformed bool #%c\n", source[i+1]);
    exit(1);
  }
}

Expr *parseIntExpr(char *source, size_t *ip) {
  size_t i = *ip;
  size_t j = i + 1;
  while (source[j] >= '0' && source[j] <= '9') {
    j++;
  }
  char *val = malloc((j - i + 1) * sizeof(char));
  strncpy(val, source + i, j - i);
  val[j - i] = '\0';
  int *int_val = malloc(sizeof(int));
  *int_val = atoi(val);
  free(val);
  *ip = j;
  return makeValExpr(IntVal, int_val);
}

Expr *parseIdentifier(char *source, size_t *ip) {
  size_t i = *ip;
  size_t j = i + 1;
  while (identChar(source[j])) {
    j++;
  }
  char *ident = malloc((j - i + 1) * sizeof(char));
  strncpy(ident, source + i, j - i);
  ident[j - i] = '\0';
  *ip = j;

  return makeIdentExpr(ident);
}

Expr *parseValue(char *source, size_t *ip) {
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

  if (source[i] == '"') {
    return parseStringValue(source, ip);
  }

  if (source[i] == '#') {
    return parseBool(source, ip);
  }

  if (source[i] >= '0' && source[i] <= '9') {
    return parseIntExpr(source, ip);
  }

  if (identChar(source[i])) {
    return parseIdentifier(source, ip);
  }

  printf("Unknown or unimplemented value type %d\n", source[i]);
  exit(1);
}

char *readSource(char *filename) {
  FILE *fp = fopen(filename, "r");
  if (fp == NULL) {
    printf("Error opening file %s\n", filename);
    exit(1);
  }
  fseek(fp, 0, SEEK_END);
  size_t size = ftell(fp);
  fseek(fp, 0, SEEK_SET);
  char *source = malloc(size + 1);
  fread(source, 1, size, fp);
  source[size] = '\0';
  fclose(fp);
  return source;
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



void *getValExprValue(Expr *expr) {
  return ((Val *)(expr->subexprs))->val;
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

void freeValue(Val *val) {
  free(val->val);
  free(val);
}

/*
 * must be passed Expr ptr
 */
void freeExpr(void *ptr) {
  Expr *expr = (Expr *)ptr;
  //static int c = 0;
  //printf("freeing expr %d etype %d\n", ++c, expr->etype);
  switch (expr->etype) {
    case ValExpr:
      //static int d = 0;
      //printf("\tfreeing val expr %d vtype %d\n", ++d, ((Val *)expr->subexprs)->vtype);
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

void reverseList(List **list) {
  if (*list == NULL) return;

  List *prev = NULL;
  List *curr = *list;
  List *next = NULL;
  //size_t size = (*list)->size;
  while (curr != NULL) {
    next = curr->tail;
    curr->tail = prev;
    prev = curr;
    curr = next;
  }
  *list = prev;

  // while (prev != NULL) {
  //   prev->size = size--; //set and dec.
  //   prev = prev->tail;
  // }
}

int listSize(List *list) {
  int size = 0;
  while (list != NULL) {
    size++;
    list = list->tail;
  }
  return size;
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

Expr *parseExpr(char *source, size_t *ip);

Expr *trySpecialForm(char *ident, char *expectIdent, size_t expectArgs,
  List *args, Expr *(*makeExpr)(Expr *, Expr *)) {
  
  if (strcmp(ident, expectIdent) == 0) {
    if (listSize(args) != expectArgs) {
      printf("Error: %s needs %d arguments. Given %d\n", expectIdent, expectArgs, listSize(args));
      exit(1);
    }
    return makeExpr(args->head, args->tail->head);
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
    if ((specialExpr = trySpecialForm(ident, "decl", 2, args, &makeDeclExpr)) != NULL ||
        (specialExpr = trySpecialForm(ident, "fun", 2, args, &makeFunExpr)) != NULL ||
        (specialExpr = trySpecialForm(ident, "if", 3, args, &makeIfExpr)) != NULL) { // this is just broken until I refactor to makeNExpr TODO: fix
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

void freeScope(Scope *scope) {
  //freeTrie(scope->identifiers);
  freeTrieWith(scope->identifiers, &freeExpr);
  free(scope);
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

int evalExpr(Expr *expr, Scope *scope) {
  switch (expr->etype) {
    case TypeExpr: //fallthrough
    case ValExpr:
    case IdentExpr:
    case ApplyExpr:
    case ListExpr:
    case TypedValExpr: // temporary?
    case ReturnExpr:
    case DeclExpr:
      emitExpr(expr, scope);
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


  // printf("ident: %s\n", ident);
  // printf("types: ");
  // List *curr = types;
  // while (curr != NULL) {
  //   printf("\"");
  //   printExpr(curr->head);
  //   printf("\",");
  //   curr = curr->tail;
  // }
  // printf("\n");


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