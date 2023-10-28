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
void print_list(List *list) {
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

Trie *gloabal_identifiers;
void initialize_gloabal_identifiers() {
  // this will cause a leak if called more than once (without freeing)
  gloabal_identifiers = makeTrie();
  trieAdd(gloabal_identifiers, "printf", makeValExpr(ValExpr, NULL));
  trieAdd(gloabal_identifiers, "scanf", makeValExpr(ValExpr, NULL));
  trieAdd(gloabal_identifiers, "add", makeValExpr(ValExpr, NULL));
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

  if (source == NULL || source[i] == '\0') {
    *ip = i;
    return NULL;
  }


  //parse string
  if (source[i] == '"') {
    size_t j = i + 1;
    while (source[j] != '"') {
      j++;
    }
    char *val = malloc(j - i);
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
    char *val = malloc(j - i);
    strncpy(val, source + i, j - i);
    val[j - i] = '\0';
    int *int_val = malloc(sizeof(int));
    *int_val = atoi(val);
    *ip = j;
    return makeValExpr(IntVal, int_val);
  }

  //parse identifier
  if (identChar(source[i])) {
    size_t j = i + 1;
    while (identChar(source[j])) {
      j++;
    }
    char *ident = malloc(j - i);
    strncpy(ident, source + i, j - i);
    ident[j - i] = '\0';
    *ip = j;
    return makeIdentExpr(ident);
  }

  printf("Unknown or unimplemented value type %d\n", source[i]);
  exit(1);
}

char *read_source(char *filename) {
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

void print_val_expr(Expr *expr) {
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

void print_ident_expr(Expr *expr) {
  printf("%s\n", (char *)expr->subexprs);
}

void print_expr(Expr *expr) {
  if (expr == NULL) {
    printf("NULL\n");
    return;
  }
  switch (expr->etype) {
    case ValExpr:
      print_val_expr(expr);
      break;
    case IdentExpr:
      print_ident_expr(expr);
      break;
    default:
      printf("Unknown or unimplemented expr type\n");
      break;
  }

}


int main(char argc, char **argv) {
  // char *a = "hello";
  // char *b = "world";
  // int *c = malloc(sizeof(int));
  // *c = 42;

  // List *list = malloc(3 * sizeof(List));
  // list[0].head = a;
  // list[0].tail = &list[1];
  // list[1].head = b;
  // list[1].tail = &list[2];
  // list[2].head = c;
  // list[2].tail = NULL;


  // List *l = makeList("1", makeList("2", makeList("3", NULL)));
  // print_list(l);
  // freeList(l);

  // List *l2;

  char *source = read_source(argv[1]);
  size_t si = 0;
  while (source[si] != '\0') {
    Expr *expr = parseValue(source, &si);
    print_expr(expr);
  }

  free(source);
}