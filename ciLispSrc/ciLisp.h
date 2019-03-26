// Eduard Bolanos
// comp232
// ciLisp Project
// Completed ciLisp without any errors, using test cases provided to check everything works properly.
#ifndef __cilisp_h_
#define __cilisp_h_

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>

#include "ciLispParser.h"

int yyparse(void);

int yylex(void);

void yyerror(char *);

typedef enum oper { // must be in sync with funcs in resolveFunc()
    NEG,
    ABS,
    EXP,
    SQRT,
    ADD,
    SUB,
    MULT,
    DIV,
    REMAINDER,
    LOG,
    POW,
    MAX,
    MIN,
    EXP2,
    CBRT,
    HYPOT,
    PRINT,
    READ,
    RAND,
    EQUAL,
    SMALLER,
    LARGER,
    INVALID_OPER=255
} OPER_TYPE;

typedef enum {
    NUM_TYPE, FUNC_TYPE, SYM_TYPE, COND_TYPE
} AST_NODE_TYPE;

typedef enum { VARIABLE_TYPE, LAMBDA_TYPE, ARG_TYPE
} SYMBOL_TYPE;

typedef enum { NO_TYPE, INTEGER_TYPE, REAL_TYPE
} DATA_TYPE;

typedef struct {
    double value;
} NUMBER_AST_NODE;

typedef struct {
    char *name;
    struct ast_node *opList;
} FUNCTION_AST_NODE;

typedef  struct symbol_ast_node{
    char *name;
}SYMBOL_AST_NODE;

typedef struct stack_node {
    struct ast_node *val;
    struct stack_node *next;
} STACK_NODE;

typedef struct symbol_table_node {
    SYMBOL_TYPE type;
    DATA_TYPE val_type;
    char *ident;
    struct ast_node *val;
    STACK_NODE *stack;
    struct symbol_table_node *next;
} SYMBOL_TABLE_NODE;

typedef struct return_value {
    DATA_TYPE type;
    double value;
} RETURN_VALUE;

typedef struct {
    struct ast_node *cond;
    struct ast_node *zero;
    struct ast_node *nonzero;
} COND_AST_NODE;

typedef struct ast_node {
    AST_NODE_TYPE type;
    SYMBOL_TABLE_NODE *scope;
    struct ast_node *parent;
    union {
        NUMBER_AST_NODE number;
        FUNCTION_AST_NODE function;
        COND_AST_NODE condition;
        SYMBOL_AST_NODE symbol;
    } data;
    struct ast_node *next;
} AST_NODE;

AST_NODE *number(double value);

AST_NODE *function(char *funcName, AST_NODE *list);

void freeNode(AST_NODE *p);

AST_NODE *symbol(char *symbolName);

AST_NODE *setScope(SYMBOL_TABLE_NODE *scope, AST_NODE *s_expr);

SYMBOL_TABLE_NODE *let_elem(char *type, char *symbolName, SYMBOL_TABLE_NODE *arg_list, AST_NODE *symbolVal);

SYMBOL_TABLE_NODE *let_list(SYMBOL_TABLE_NODE *list, SYMBOL_TABLE_NODE *elem);

AST_NODE *expr_list(AST_NODE *list, AST_NODE *expr);

AST_NODE *condition(AST_NODE *condition, AST_NODE *expr1, AST_NODE *expr2);

SYMBOL_TABLE_NODE *arg_list(SYMBOL_TABLE_NODE *list, SYMBOL_TABLE_NODE *arg);
SYMBOL_TABLE_NODE *arg_elem(char * name);


RETURN_VALUE eval(AST_NODE *ast);


#endif

