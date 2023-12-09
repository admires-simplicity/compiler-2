#include <stdbool.h>
#include <string.h>

#include "parser.h"
#include "expr.h"

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
