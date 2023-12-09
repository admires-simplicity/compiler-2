#include <stdlib.h>

#include "value.h"

Val *makeVal(ValType type, void *val) {
  Val *v = malloc(sizeof(Val));
  v->vtype = type;
  v->val = val;
  return v;
}

void freeValue(Val *val) {
  free(val->val);
  free(val);
}