#include <string.h>
#include <stdbool.h>
#include <assert.h>

#include "identifiers.h"
#include "trie.h"
#include "expr.h"

Trie *gloabalIdentifiers = NULL;

bool reserved(char *ident) {
  //change this to checking if ident is in trie
  return strcmp(ident, "return") == 0;
}

//Trie *gloabalIdentifiers;
void initializeGloabalIdentifiers() {
  // this will cause a leak if called more than once (without freeing)
  assert(gloabalIdentifiers == NULL);
  gloabalIdentifiers = makeTrie();
  trieAdd(gloabalIdentifiers, "printf", makeValExpr(ValExpr, NULL));
  trieAdd(gloabalIdentifiers, "scanf", makeValExpr(ValExpr, NULL));
  trieAdd(gloabalIdentifiers, "add", makeValExpr(ValExpr, NULL));
}

