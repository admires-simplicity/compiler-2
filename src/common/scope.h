#ifndef SCOPE_H
#define SCOPE_H

#include "trie.h"

typedef struct Scope Scope;
struct Scope {
  Trie *identifiers;
  Scope *parent;
};

void freeScope(Scope *scope);

#endif