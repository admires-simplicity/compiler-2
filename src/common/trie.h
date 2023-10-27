#ifndef TRIE_H
#define TRIE_H

#include <stddef.h>

typedef struct Trie Trie;

struct Trie {
  char key_char;
  void *value;  
  size_t children_count;
  Trie **children;
};

Trie *makeTrie();
void freeTrie(Trie *trie);
void *trieGet(Trie *trie, char *key);
Trie *trieAdd(Trie *trie, char *key, void *value);

void print_trie(Trie *trie, int indent);

#endif