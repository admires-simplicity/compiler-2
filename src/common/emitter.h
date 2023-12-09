#ifndef EMITTER_H
#define EMITTER_H

#include "expr.h"
#include "scope.h"

void emitExpr(Expr *expr, Scope *scope);

#endif