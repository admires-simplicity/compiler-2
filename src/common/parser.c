#include <stdbool.h>
#include <string.h>

#include "parser.h"
#include "expr.h"
#include "value.h"
#include "list.h"
#include "identifiers.h"

/* Copy contents of file into string.
 */
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
