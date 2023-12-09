#ifndef IDENTIFIERS_H
#define IDENTIFIERS_H

#include <stdbool.h>

#include "trie.h"


bool reserved(char *ident);

extern Trie *gloabalIdentifiers;
void initializeGloabalIdentifiers();


#endif