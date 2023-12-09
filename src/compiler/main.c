#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>
#include <stdarg.h>

#include "../common/trie.h"
#include "../common/list.h"

#include "../common/expr.h"
#include "../common/parser.h"
#include "../common/compiler.h"

int main(char argc, char **argv) {
  if (argc != 2) {
    printf("Usage: %s <filename>\n", argv[0]);
    exit(1);
  }

  List *program = NULL;

  char *source = readSource(argv[1]);

  size_t si = 0;
  while (source[si] != '\0') {
    si = skipWhitespace(source, si);
    Expr *expr = parseExpr(source, &si);
    if (expr != NULL) program = makeList(expr, program);
  }

  compile(&program);

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