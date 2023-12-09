#include <stdio.h>

#include "expr.h"
#include "value.h"
#include "printer.h"

void printValExpr(Expr *expr) {
  switch (((Val *)(expr->subexprs))->vtype) {
    case IntVal:
      printf("%d", *(int *)(((Val *)(expr->subexprs))->val));
      break;
    case BoolVal:
      printf("%s", *(int *)(((Val *)(expr->subexprs))->val) ? "#t" : "#f");
      break;
    case StringVal:
      printf("\"%s\"", (char *)(((Val *)(expr->subexprs))->val));
      break;
    default:
      printf("Unknown or unimplemented val type %d\n", ((Val *)(expr->subexprs))->vtype);
      break;
  }
}

// void printIdentExpr(Expr *expr) {
//   printf("%s", (char *)expr->subexprs);
// }

// void printExpr(Expr *expr) {
//   //static int c = 0;
//   //printf("[expr %d etype %d]: ", ++c, expr->etype);
//   if (expr == NULL) {
//     printf("NULL\n");
//     return;
//   }
//   switch (expr->etype) {
//     case ValExpr:
//       printValExpr(expr);
//       break;
//     case IdentExpr:
//       printIdentExpr(expr);
//       break;
//     case ApplyExpr:
//       static int level = 0;
//       level++;
//       Expr *func = getExprSubexpr(expr, 0);
//       Expr *args = getExprSubexpr(expr, 1);
//       List *argList = args->subexprs;
//       printf("(");
//       printExpr(func);
//       //printf(" [expr %d etype %d]: ", ++c, args->etype);
//       while (argList != NULL) {
//         printf(" ");
//         printExpr(argList->head);
//         argList = argList->tail;
//       }
//       --level;
//       printf(")");
//       if (level == 0) printf("\n");
//       break;
//     default:
//       printf("(%s): Unknown or unimplemented expr type %d\n", __func__, expr->etype);
//       break;
//   }
// }