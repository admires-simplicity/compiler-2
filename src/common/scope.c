#include "scope.h"
#include "expr.h"
#include "trie.h"
#include "identifiers.h"

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
