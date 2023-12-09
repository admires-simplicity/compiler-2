#ifndef VALUE_H
#define VALUE_H

typedef enum {
  IntVal,
  BoolVal,
  StringVal,
} ValType;

typedef struct {
  ValType vtype;
  void *val;
} Val;

Val *makeVal(ValType type, void *val);
void freeValue(Val *val);


#endif