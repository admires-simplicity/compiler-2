#ifndef UTILITY_H
#define UTILITY_H

#include <stdbool.h>

#include "list.h"
#include "expr.h"

void map(List *list, void (*f)(Expr *));
List *gatherFrom(List *list, bool (*gather)(Expr *));

#endif