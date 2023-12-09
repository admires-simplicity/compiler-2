#ifndef SCOPE_H
#define SCOPE_H

#include "trie.h"

typedef struct Scope Scope;
struct Scope {
  Trie *identifiers;
  Scope *parent;
};

Scope *makeScope(Scope *parent, Trie *identifiers);
Scope *makeGlobalScope();
Scope *makeLocalScope(Scope *parent);
void freeScope(Scope *scope);

#endif