#include "scope.h"
#include "expr.h"

void freeScope(Scope *scope) {
  //freeTrie(scope->identifiers);
  freeTrieWith(scope->identifiers, &freeExpr);
  free(scope);
}
