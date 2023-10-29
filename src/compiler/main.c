#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>

#include "../common/trie.h"

typedef enum {
  IntVal,
  BoolVal,
  StringVal,
} ValType;

typedef struct {
  ValType vtype;
  void *val;
} Val;

typedef enum {
  ValExpr,
  IdentExpr,
  ApplyExpr,
  TypeExpr,
  ListExpr,
} ExprType;

typedef struct {
  ExprType etype;
  size_t size;
  void *subexprs;
} Expr;

typedef struct List List;
struct List {
  size_t size;
  void *head;
  List *tail;
};

List *makeList(void *head, List *tail) {
  List *list = malloc(sizeof(List));
  list->size = tail == NULL ? 1 : tail->size + 1;
  list->head = head;
  list->tail = tail;
  return list;
}

void freeList(List *list) {
  if (list->tail != NULL) {
    freeList(list->tail);
  }
  //free(list->head); // is this right?
  free(list);
}

/* Prints a list. Only works for char* list.
 */
void printList(List *list) {
  printf("[");
  while (list != NULL) {
    printf("%s", (char *)list->head);
    if (list->tail != NULL) {
      printf(", ");
    }
    list = list->tail;
  }
  printf("]\n");
}

Val *makeVal(ValType type, void *val) {
  Val *v = malloc(sizeof(Val));
  v->vtype = type;
  v->val = val;
  return v;
}

Expr *makeValExpr(ValType type, void *val) {
  Expr *expr = malloc(sizeof(Expr));
  expr->etype = ValExpr;
  expr->size = 1;
  expr->subexprs = makeVal(type, val);
  return expr;
}

Expr *makeIdentExpr(char *ident) {
  Expr *expr = malloc(sizeof(Expr));
  expr->etype = IdentExpr;
  expr->size = 1;
  expr->subexprs = ident;
  return expr;
}

Expr *makeApplyExpr(Expr *func, Expr *args) {
  assert(args->etype == ListExpr);
  Expr *expr = malloc(sizeof(Expr));
  expr->etype = ApplyExpr;
  expr->size = sizeof(Expr *) * 2;
  expr->subexprs = malloc(expr->size);
  ((Expr **)expr->subexprs)[0] = func;
  ((Expr **)expr->subexprs)[1] = args;
  return expr;
}

Expr *makeListExpr(List *list) {
  Expr *expr = malloc(sizeof(Expr));
  expr->etype = ListExpr;
  expr->size = list->size;
  // List **list_ptr = malloc(sizeof(List *));
  // *list_ptr = list;
  // expr->subexprs = list_ptr;
  expr->subexprs = list;
  return expr;
}

Trie *gloabalIdentifiers;
void initializeGloabalIdentifiers() {
  // this will cause a leak if called more than once (without freeing)
  gloabalIdentifiers = makeTrie();
  trieAdd(gloabalIdentifiers, "printf", makeValExpr(ValExpr, NULL));
  trieAdd(gloabalIdentifiers, "scanf", makeValExpr(ValExpr, NULL));
  trieAdd(gloabalIdentifiers, "add", makeValExpr(ValExpr, NULL));
}

size_t skipWhitespace(char *source, size_t i) {
  while (source[i] == ' ' || source[i] == '\n' || source[i] == '\t') {
    ++i;
  }
  return i;
}

bool identChar(char c) {
  return c != ' ' && c != '\n' && c != '\t' && c != '(' && c != ')' && c != '\0';
}



Expr *parseValue(char *source, size_t *ip) {
  size_t i = *ip;

  i = skipWhitespace(source, i);

  if (source == NULL) {
    printf("Error: source is NULL\n");
    exit(1);
  }

  if (source[i] == '\0') {
    *ip = i;
    return NULL;
  }


  //parse string
  if (source[i] == '"') {
    size_t j = i + 1;
    while (source[j] != '"') {
      j++;
    }
    char *val = malloc((j - i + 1) * sizeof(char));
    strncpy(val, source + i + 1, j - i - 1);
    val[j - i - 1] = '\0';
    *ip = j + 1;
    return makeValExpr(StringVal, val);
  }

  //parse bool
  if (source[i] == '#') {
    if (source[i + 1] == 't') {
      *ip = i + 2;
      return makeValExpr(BoolVal, (void *)1);
    }
    if (source[i + 1] == 'f') {
      *ip = i + 2;
      return makeValExpr(BoolVal, (void *)0);
    } 
  }

  //parse int
  if (source[i] >= '0' && source[i] <= '9') {
    size_t j = i + 1;
    while (source[j] >= '0' && source[j] <= '9') {
      j++;
    }
    char *val = malloc((j - i + 1) * sizeof(char));
    strncpy(val, source + i, j - i);
    val[j - i] = '\0';
    int *int_val = malloc(sizeof(int));
    *int_val = atoi(val);
    free(val);
    *ip = j;
    return makeValExpr(IntVal, int_val);
  }

  //parse identifier
  if (identChar(source[i])) {
    size_t j = i + 1;
    while (identChar(source[j])) {
      j++;
    }
    char *ident = malloc((j - i + 1) * sizeof(char));
    strncpy(ident, source + i, j - i);
    ident[j - i] = '\0';
    *ip = j;
    return makeIdentExpr(ident);
  }

  printf("Unknown or unimplemented value type %d\n", source[i]);
  exit(1);
}

char *readSource(char *filename) {
  FILE *fp = fopen(filename, "r");
  if (fp == NULL) {
    printf("Error opening file %s\n", filename);
    exit(1);
  }
  fseek(fp, 0, SEEK_END);
  size_t size = ftell(fp);
  fseek(fp, 0, SEEK_SET);
  char *source = malloc(size + 1);
  fread(source, 1, size, fp);
  source[size] = '\0';
  fclose(fp);
  return source;
}

void printValExpr(Expr *expr) {
  // //switch ((ValType)expr->subexprs) {
  // // switch (((Expr *)expr->subexprs)->etype) {
  // switch ((Val *((Expr *)expr->subexprs)->vtype) {
  //   case IntVal:
  //     printf("%d\n", *((int *)expr->subexprs));
  //     break;
  //   case BoolVal:
  //     printf("%s\n", *((int *)expr->subexprs) ? "#t" : "#f");
  //     break;
  //   case StringVal:
  //     printf("\"%s\"\n", (char *)expr->subexprs);
  //     break;
  //   default:
  //     printf("Unknown or unimplemented val type %d\n", (long int)expr->subexprs);
  //     break;
  // }
  switch (((Val *)(expr->subexprs))->vtype) {
    case IntVal:
      printf("%d\n", *(int *)(((Val *)(expr->subexprs))->val));
      break;
    case BoolVal:
      printf("%s\n", *(int *)(((Val *)(expr->subexprs))->val) ? "#t" : "#f");
      break;
    case StringVal:
      printf("\"%s\"\n", (char *)(((Val *)(expr->subexprs))->val));
      break;
    default:
      printf("Unknown or unimplemented val type %d\n", ((Val *)(expr->subexprs))->vtype);
      break;
  }
}

void printIdentExpr(Expr *expr) {
  printf("%s\n", (char *)expr->subexprs);
}

void printExpr(Expr *expr) {
  if (expr == NULL) {
    printf("NULL\n");
    return;
  }
  switch (expr->etype) {
    case ValExpr:
      printValExpr(expr);
      break;
    case IdentExpr:
      printIdentExpr(expr);
      break;
    default:
      printf("Unknown or unimplemented expr type\n");
      break;
  }

}

void freeValue(Val *val) {
  free(val->val);
  free(val);
}

void freeExpr(Expr *expr) {
  switch (expr->etype) {
    case ValExpr:
      freeValue(expr->subexprs);
      break;
    case IdentExpr:
      free(expr->subexprs);
      break;
    case ApplyExpr:
      freeExpr(((Expr **)expr->subexprs)[0]);
      freeExpr(((Expr **)expr->subexprs)[1]);
      free(expr->subexprs);
      break;
    case ListExpr:
      freeList(expr->subexprs);
      break;
    default:
      printf("Unknown or unimplemented expr type\n");
      break;
  }
  free(expr);
}

void freeExprList(List *list) {
  if (list->tail != NULL) {
    freeExprList(list->tail);
  }
  freeExpr(list->head);
  free(list);
}

void reverseList(List **list) {
  List *prev = NULL;
  List *curr = *list;
  List *next = NULL;
  while (curr != NULL) {
    next = curr->tail;
    curr->tail = prev;
    prev = curr;
    curr = next;
  }
  *list = prev;
}

void map(List *list, void (*f)(Expr *)) {
  while (list != NULL) {
    f(list->head);
    list = list->tail;
  }
}

void println(char *str) {
  printf("%s\n", str);
}

int main(char argc, char **argv) {
  if (argc != 2) {
    printf("Usage: %s <filename>\n", argv[0]);
    exit(1);
  }

  List *program = NULL;

  char *source = readSource(argv[1]);
  size_t si = 0;
  while (source[si] != '\0') {
    Expr *expr = parseValue(source, &si); 
    if (expr != NULL) program = makeList(expr, program);
  }

  reverseList(&program);
  map(program, &printExpr);

  freeExprList(program);

  free(source);
}