#ifndef PARSER_H
#define PARSER_H

#include <stdio.h>
#include <stdbool.h>

#include "expr.h"

char *readSource(char *filename);

size_t skipComment(char *source, size_t i);
size_t skipWhitespace(char *source, size_t i);
bool identChar(char c);
Expr *parseStringValue(char *source, size_t *ip);
Expr *parseBool(char *source, size_t *ip);
Expr *parseIntExpr(char *source, size_t *ip);
Expr *parseIdentifier(char *source, size_t *ip);
Expr *parseValue(char *source, size_t *ip);
Expr *parseApplyExpr(char *source, size_t *ip);
Expr *parseExpr(char *source, size_t *ip);


#endif