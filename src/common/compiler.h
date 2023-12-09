#ifndef COMPILER_H
#define COMPILER_H

#include "expr.h"
#include "scope.h"

#include "list.h"

int evalExpr(Expr *expr, Scope *scope);

int compile(List **program);


#endif