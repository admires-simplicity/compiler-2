#ifndef LIST_H
#define LIST_H

typedef struct List List;
struct List {
  void *head;
  List *tail;
};

List *makeList(void *head, List *tail);
void freeList(List *list);
void freeListWith(List *list, void (*freeValue)(void *));
void printList(List *list);
int listSize(List *list);
void reverseList(List **list);

void map(List *list, void (*f)(Expr *));


#endif