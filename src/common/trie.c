#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "trie.h"

Trie *makeTrie() {
  Trie *trie = malloc(sizeof(Trie));
  if (trie == NULL) {
    printf("Error: Could not allocate memory for Trie\n");
    return NULL;
  }
  trie->key_char = 0;
  trie->value = NULL;
  trie->children_count = 0;
  trie->children = NULL;
  return trie;
}

void freeTrie(Trie *trie) {
  if (trie->children != NULL) {
    for (size_t i = 0; i < trie->children_count; ++i) {
      freeTrie(trie->children[i]);
    }
    free(trie->children);
  }
  free(trie);
}


void *trieGet(Trie *trie, char *key) {
  if (trie == NULL || key == NULL) { // invalid
    return NULL;
  }
  if (key[0] == '\0') {
    return trie->value; // NULL if key is empty in trie
  }
  for (size_t i = 0; i < trie->children_count; ++i) {
    if (trie->children[i]->key_char == key[0]) {
      return trieGet(trie->children[i], key + 1);
    }
  }
  return NULL; // key not in trie
}

Trie *trieAdd(Trie *trie, char *key, void *value) {
  if (trie == NULL || key == NULL) {
    // invalid
    return NULL;
  }

  if (key[0] == '\0') {
    // we are at trie[key]
    if (trie->value != NULL) return NULL; // key already in trie
    trie->value = value;
    return trie;
  }

  // key_char in trie
  for (int i = 0; i < trie->children_count; ++i) {
    if (trie->children[i]->key_char == key[0]) {
      return trieAdd(trie->children[i], key + 1, value);
    }
  }

  // key_char not in trie
  size_t key_len = strlen(key);
  for (size_t i = 0; i < key_len; ++i) {
    Trie **new_children = realloc(
      trie->children, sizeof(Trie) * (trie->children_count + 1));
    if (new_children == NULL) {
      printf("Error: Could not reallocate memory for Trie->children\n");
      exit(1);
    }
    trie->children = new_children;
    ++trie->children_count;
    trie->children[trie->children_count - 1] = makeTrie();
    trie->children[trie->children_count - 1]->key_char = key[i];
    trie = trie->children[trie->children_count - 1];
  }
  trie->value = value;
  return trie;
}

void print_trie(Trie *trie, int indent) {
  if (trie == NULL) {
    printf("NullTrie\n");
    return;
  }
  for (int i = 0; i < indent; ++i) {
    printf("-");
  }
  printf("Key: %c\n", trie->key_char);
  if (trie->value != NULL) {
    for (int i = 0; i < indent; ++i) {
      printf(" ");
    }
    printf("Value: %s\n", (char *) trie->value);
  }
  for (size_t i = 0; i < trie->children_count; ++i) {
    print_trie(trie->children[i], indent + 1);
  }
}