// Eduard Bolanos
// comp232
// ciLisp Project
// Completed ciLisp without any errors, using test cases provided to check everything works properly.

#include "ciLisp.h"
#include "string.h"
#include <stdlib.h>

void yyerror(char *s) {
    printf("\nERROR: %s\n", s);
    // note stderr that normally defaults to stdout, but can be redirected: ./src 2> src.log
    // CLion will display stderr in a different color from stdin and stdout
}

//
// find out which function it is
int resolveFunc(char *func)
{
    char *funcs[] = {"neg", "abs", "exp", "sqrt", "add", "sub", "mult", "div", "remainder", "log", "pow", "max",
                     "min", "exp2", "cbrt", "hypot", "print", "read", "rand", "equal", "smaller", "larger", ""};

    int i = 0;
    while (funcs[i][0] != '\0')
    {
        if (strcmp(funcs[i], func) == 0)
            return i;

        i++;
    }
    //yyerror("invalid function\n");
    return INVALID_OPER;
}

//
// create a node for a number
AST_NODE *number(double value)
{
    AST_NODE *p;
    size_t nodeSize;

    // allocate space for the fixed sie and the variable part (union)
    nodeSize = (sizeof(AST_NODE) + sizeof(SYMBOL_AST_NODE));
    if((p = calloc(1,nodeSize)) == NULL)
        yyerror("Out of memory\n");

    p->type = NUM_TYPE;
    p->data.number.value = value;

    return p;
}

//
// create a node for a function
AST_NODE *function(char *funcName, AST_NODE *list)
{
// Added functionality for taking in user input, 'read', and ability to add randomness 'rand' which is
// allowed in the grammar.
    if(strcmp(funcName, "rand") == 0)
    {
        double numNode = (double)(rand()); //NOLINT
        fprintf(stderr, "rand : %lf\n", numNode);
        return number(numNode);
    }
    else if(strcmp(funcName, "read") == 0)
    {
        printf("read := ");
        size_t size = 0;
        char * line = NULL;

        getline(&line, &size, stdin);
        double userInput = strtod(line, NULL);

        free(line);

        return number(userInput);
    }


    AST_NODE *p;
    size_t nodeSize;

// Allocate space for the fixed size and the variable part (union)
    nodeSize = (sizeof(AST_NODE) + sizeof(SYMBOL_AST_NODE));
    if((p = calloc(1,nodeSize)) == NULL)
        yyerror("Out of memory\n");

// Removed op1 and op2, introduced opList that allows unlimited amount of operations.
    p->type = FUNC_TYPE;
    p->data.function.name = funcName;
    p->data.function.opList = list;

// Sets the appropriate parent to the list.
    while(list != NULL)
    {
        list->parent = p;
        list = list->next;
    }

    return p;
}

//
// free a node
void freeNode(AST_NODE *p)
{
    if (!p)
        return;

    if (p->type == FUNC_TYPE)
    {
        free(p->data.function.name);
        freeNode(p->data.function.opList);
    }

    free(p);
}

//
// setting the symbols
AST_NODE *symbol(char *name)
{
    AST_NODE *somePointer;
    size_t nodeSize = (sizeof(AST_NODE) + sizeof(SYMBOL_AST_NODE));
    if((somePointer = calloc(1,nodeSize)) == NULL)
        yyerror("Out of memory\n");

    somePointer->type = SYM_TYPE;
    somePointer->data.symbol.name = name;

    return somePointer;
}

//
// Creates/sets the scope for the grammar
AST_NODE *setScope(SYMBOL_TABLE_NODE *scope, AST_NODE *s_expr)
{
    s_expr->scope = scope;
    return s_expr;
}


//
// Works in a similar fashion to resolveFunc except this is for types, ('integer', 'real' or nothing)
int resolveType(char *type)
{
    if (type == NULL)
        return NO_TYPE;

    if (strcmp("integer", type) == 0)
        return INTEGER_TYPE;

    if (strcmp("real", type) == 0)
        return REAL_TYPE;

    yyerror("Invalid Type");
    return NO_TYPE;
}

//
// Edited to include what kind of types are resolved, ('integer', 'real' or nothing).
// Edited #2 added the implementation of lambda type into the grammar, as well letting let_elem taking in and arg_list
SYMBOL_TABLE_NODE *let_elem(char *type, char *symbolName, SYMBOL_TABLE_NODE *arg_list, AST_NODE *symbolVal)
{
    SYMBOL_TABLE_NODE *pointer = calloc(1, sizeof(SYMBOL_TABLE_NODE));

    pointer->ident = symbolName;
    pointer->val = symbolVal;
    pointer->val_type = resolveType(type);

    if (arg_list == NULL)
        pointer->type = VARIABLE_TYPE;
    else {
        pointer->type = LAMBDA_TYPE;
        pointer->next = arg_list;
        if (pointer->val->scope == NULL)
            pointer->val->scope = pointer->next;
        else {
            SYMBOL_TABLE_NODE * iter = pointer->val->scope;
            while (iter->next != NULL)
                iter = iter->next;
            iter->next = pointer->next;
        }
    }
    return  pointer;
}

bool duplicateSymbol(SYMBOL_TABLE_NODE * iterator, SYMBOL_TABLE_NODE * elem)
{
    if(strcmp(iterator->ident, elem->ident) == 0)
    {
        yyerror("Error, that symbol already exists\n");
        return true;
    }
    return false;
}

//
// Let_list means that there will be multiple symbol tables associated with
// the roots of the expressions within which they are defined.
SYMBOL_TABLE_NODE *let_list(SYMBOL_TABLE_NODE *list, SYMBOL_TABLE_NODE *elem)
{
    if(list == NULL)
        return elem;
    SYMBOL_TABLE_NODE *iterator = list;
    while(iterator->next != NULL)
    {
        if (duplicateSymbol(iterator, elem))
            return list;
        iterator = iterator->next;
    }
    if (duplicateSymbol(iterator, elem))
        return list;
    iterator->next = elem;
    return list;
}

//
// It is structured in a similar fashion to let_list except now it expands the capability of
// ciLisp by adding support for parameter lists of arbitrary length.
AST_NODE *expr_list(AST_NODE *list, AST_NODE *expr)
{
    if (list == NULL)
        return expr;
    AST_NODE * iter = list;
    while (iter->next != NULL)
        iter = iter->next;
    iter->next = expr;
    return list;
}

//
// Works in a similar fashion to let_elem in where it sets the correct elements for the argument list
SYMBOL_TABLE_NODE *arg_elem(char * name)
{
    SYMBOL_TABLE_NODE * node = calloc(1, sizeof(SYMBOL_TABLE_NODE));
    node->ident = name;
    node->val_type = NO_TYPE;
    node->type = ARG_TYPE;
    return node;
}

//
// A way to set the argument list correctly so it does not return null or break completely
SYMBOL_TABLE_NODE *arg_list(SYMBOL_TABLE_NODE *list, SYMBOL_TABLE_NODE *arg)
{
    if (list == NULL)
        return arg;
    SYMBOL_TABLE_NODE * i = list;
    while (i->next != NULL)
        i = i->next;
    i->next = arg;
    return list;
}

//
// sets the parent node
void setParentNode(AST_NODE *p)
{
    AST_NODE *iterator = p->data.function.opList;

    while(iterator != NULL)
    {
        iterator->parent = p;
        iterator = iterator->next;
    }
}

//
// checking whether a symbol is valid or not valid
RETURN_VALUE evalSymbols(char *name, AST_NODE *p)
{
    RETURN_VALUE value;

    SYMBOL_TABLE_NODE *node = p->scope;
    bool foundNode = false;
    while(!foundNode && node != NULL)
    {
        if(strcmp(node->ident, name) == 0)
        {
            foundNode = true;
            if (node->type == ARG_TYPE)
                value = eval(node->stack->val);
            else
                value = eval(node->val);
        }
        node = node->next;
    }

    if(!foundNode)
    {
        if(p->parent != NULL)
            value = evalSymbols(name, p->parent);
        else
            yyerror("ERROR invalid symbol.\n");
    }

    return value;
}

//
// Its a function that counts how many operands were passed which is later used in eval
int numOfOperands(AST_NODE *p)
{
    AST_NODE *node = p->data.function.opList;
    int numOfOperands = 0;

    while(node != NULL)
    {
        numOfOperands++;
        node = node->next;
    }
    return numOfOperands;
}

//
// The basic setup for the condition function. Setting the associated that if the condition holds or otherwise
AST_NODE *condition(AST_NODE *condition, AST_NODE *expr1, AST_NODE *expr2)
{
    AST_NODE *p;
    size_t nodeSize;
    nodeSize = sizeof(COND_AST_NODE);

    if((p = calloc(1,nodeSize)) == NULL)
        yyerror("Out of memory\n");

    p->type = COND_TYPE;

    p->data.condition.cond = condition;
    p->data.condition.nonzero = expr1;
    p->data.condition.zero = expr2;

    expr1->parent = condition;
    expr2->parent = condition;
    condition->parent = p;

    return p;
}

//
// Going through the scope and finding all the symbols that are within said scope
SYMBOL_TABLE_NODE *findSymbol(AST_NODE *p, char *name)
{
    if(p == NULL)
        return NULL;

    SYMBOL_TABLE_NODE *symbol = p->scope;

    if(p->scope != NULL)
    {
        while(symbol != NULL)
        {
            if(strcmp(symbol->ident,name) == 0)
            {
                return symbol;
            }
            symbol = symbol->next;
        }
    }
    return findSymbol(p->parent, name);
}

//
// Works in a similar way as numOfOperands function which basically counts the number of parameters that were
// entered
int countParameters(SYMBOL_TABLE_NODE *p) {
    SYMBOL_TABLE_NODE *node = p;

    int numOfParameters = 0;

    while (node != NULL && node->type == ARG_TYPE) {
        numOfParameters++;
        node = node->next;
    }
    return numOfParameters;

}

//
// A function that allows you to pop from the stack
void popStack(SYMBOL_TABLE_NODE *p)
{
    while(p != NULL && p->type == ARG_TYPE)
    {
        STACK_NODE *temp = p->stack;
        p->stack = p->stack->next;
        free(temp);

        p = p->next;
    }
}

//
// A function that allows you to push onto the stack
void pushStack(AST_NODE *p, SYMBOL_TABLE_NODE *goblinSlayer)
{
   while (goblinSlayer != NULL && goblinSlayer->type == ARG_TYPE)
   {
       STACK_NODE *diamond= calloc(1, sizeof(STACK_NODE));
       STACK_NODE *temp = goblinSlayer->stack;
       diamond->val = p;
       diamond->next = temp;
       goblinSlayer->stack = diamond;

       goblinSlayer = goblinSlayer->next;
       p = p->next;
   }
}

//
// A function that allows the lambda experssion to be used in the grammar, will later be called upon in eval
RETURN_VALUE lambda(AST_NODE *p)
{
    SYMBOL_TABLE_NODE *node = findSymbol(p,p->data.function.name);

    if(node == NULL) {
        yyerror("Undefined symbol");
        return (RETURN_VALUE) {NO_TYPE, 0.0};
    }

    int parameters = countParameters(node->next);
    int op = numOfOperands(p);
    if(op < parameters)
    {
        printf("ERROR: too few parameters for the function %s\n", p->data.function.name);
        return (RETURN_VALUE) {NO_TYPE, 0.0};
    }
    AST_NODE *operand = (p->data.function.opList);
    SYMBOL_TABLE_NODE *argument = (node->next);

    pushStack(operand,argument);
    RETURN_VALUE value = eval(node->val);
    popStack(argument);
    return value;
}

//
// evaluate an abstract syntax tree
// p points to the root
RETURN_VALUE eval(AST_NODE *p) {
    RETURN_VALUE value;
    value.type = NO_TYPE;
    value.value = 0.0;


    if (!p)
        return value;

    setParentNode(p);

    if (p->type == NUM_TYPE) {
        value.value = p->data.number.value;
        value.type = INTEGER_TYPE;
        if (value.type == INTEGER_TYPE) {
            value.value = round(value.value);
        }
    }
    else if (p->type == SYM_TYPE) {
        value = evalSymbols(p->data.symbol.name, p);
        value.type = value.type;
    }
    else if(p->type == COND_TYPE) {

        return (eval(p->data.condition.cond).value) ?
            eval(p->data.condition.nonzero) :
            eval(p->data.condition.zero);
     }
     else if (p->type == FUNC_TYPE) {

        int op = numOfOperands(p);

        AST_NODE *op1 = p->data.function.opList;
        AST_NODE *node = p->data.function.opList;

        if(op == 0)
        {
            printf("ERROR: too few parameters for the function %s\n", p->data.function.name);
            return (RETURN_VALUE) {NO_TYPE, 0.0};
        }
        switch (resolveFunc(p->data.function.name)) {
            case NEG:
                if(op > 2)
                {
                    printf("WARNING: too many parameters for the function \n");
                }
                value = eval(op1);
                value.value = -(value.value);
                if (value.type == INTEGER_TYPE) {
                    value.type = INTEGER_TYPE;
                }
                break;
            case ABS:
                value.value = eval(op1).value;
                value.value = fabs(value.value);
                if (eval(op1).type == INTEGER_TYPE) {
                    printf("WARNING: incompatible type assignment for variables in ABS.\n");
                }
                value.type = REAL_TYPE;
                break;
            case EXP:
                value = eval(op1);
                value.value = exp(value.value);
                if (eval(op1).type == INTEGER_TYPE) {
                    printf("WARNING: incompatible type assignment for variables in EXP.\n");
                }
                value.type = REAL_TYPE;

                break;
            case SQRT:
                if(op > 1)
                {
                    printf("WARNING: too many parameters for the function \n");
                }
                value.value = eval(op1).value;
                value.value = sqrt(value.value);
                if (eval(op1).type == INTEGER_TYPE) {
                    printf("WARNING: incompatible type assignment for variables in SQRT.\n");
                }
                value.type = REAL_TYPE;

                break;
            case ADD:

                while(node != NULL)
                {
                    RETURN_VALUE temp = eval(node);

                    if (value.type == NO_TYPE && temp.type == NO_TYPE)
                        value.type = NO_TYPE;
                    else if (value.type == INTEGER_TYPE && temp.type == INTEGER_TYPE)
                        value.type = INTEGER_TYPE;
                    else
                        value.type = REAL_TYPE;
                    value.value += temp.value;

                    node = node->next;
                }
                break;
            case SUB:
                if (eval(op1).type == INTEGER_TYPE &&
                    eval(op1->next).type == INTEGER_TYPE) {
                    value.type = INTEGER_TYPE;
                } else if (eval(op1).type == INTEGER_TYPE &&
                           eval(op1->next).type == REAL_TYPE) {
                    printf("WARNING: incompatible type assignment for variables in SUB.\n");
                    value.type = INTEGER_TYPE;
                } else {
                    value.type = REAL_TYPE;
                }
                value.value = eval(op1).value - eval(op1->next).value;
                break;
            case MULT:
                value.value = 1.0;
                while(node != NULL)
                {
                    RETURN_VALUE temp = eval(node);

                    if (value.type == NO_TYPE && temp.type == NO_TYPE)
                        value.type = NO_TYPE;
                    else if (value.type == INTEGER_TYPE && temp.type == INTEGER_TYPE)
                        value.type = INTEGER_TYPE;
                    else
                        value.type = REAL_TYPE;
                    value.value *= temp.value;

                    node = node->next;
                }
                break;
            case DIV:
                if (eval(op1).type == INTEGER_TYPE) {
                    printf("WARNING: incompatible type assignment for variables in DIV.\n");
                }
                value.value = eval(op1).value / eval(op1->next).value;
                break;
            case REMAINDER:
                value.value = remainder(eval(op1).value, eval(op1->next).value);

                break;
            case LOG:
                if (eval(op1).type == INTEGER_TYPE) {
                    printf("WARNING: incompatible type assignment for variables in LOG.\n");
                }
                if (eval(op1).value == 2.0) {
                    value.value = log2(eval(op1->next).value);
                } else if (eval(op1).value == 10) {
                    value.value = log10(eval(op1->next).value);
                }
                break;
            case POW:
                if (eval(op1).type == INTEGER_TYPE) {
                    printf("WARNING: incompatible type assignment for variables in POW.\n");
                }
                value.value = pow(eval(op1).value, eval(op1->next).value);

                break;
            case MAX:
                if (eval(op1).type == INTEGER_TYPE) {
                    printf("WARNING: incompatible type assignment for variables in MAX.\n");
                }
                value.type = REAL_TYPE;
                value.value = fmax(eval(op1).value, eval(op1->next).value);

                break;
            case MIN:
                if (eval(op1).type == INTEGER_TYPE) {
                    printf("WARNING: incompatible type assignment for variables in MIN.\n");
                }
                value.type = REAL_TYPE;
                value.value = fmin(eval(op1).value, eval(op1->next).value);
                break;
            case EXP2:
                if (eval(op1).type == INTEGER_TYPE) {
                    printf("WARNING: incompatible type assignment for variables in EXP2.\n");
                }
                value.type = REAL_TYPE;
                value.value = exp2(eval(op1).value);

                break;
            case CBRT:
                if (eval(op1).type == INTEGER_TYPE) {
                    printf("WARNING: incompatible type assignment for variables in CBRT.\n");
                }
                value.type = REAL_TYPE;
                value.value = cbrt(eval(op1).value);

                break;
            case HYPOT:
                if (eval(op1).type == INTEGER_TYPE) {
                    printf("WARNING: incompatible type assignment for variables in HYPOT.\n");
                }
                value.value = hypot(eval(op1).value, eval(op1->next).value);

                break;
            case PRINT:
                printf("=>");
                while(node != NULL)
                {
                    value = eval(node);
                    if (value.type == INTEGER_TYPE) {
                        printf(" %d", (int) value.value);
                    } else {
                        printf(" %.2lf", value.value);
                    }

                    node = node->next;
                }
                printf("\n");
                break;
            case EQUAL:
                if(eval(op1).value == eval(op1->next).value)
                    value.value = 1;
                else
                    value.value = 0;
                break;
            case SMALLER:
                if(eval(op1).value < eval(op1->next).value)
                    value.value = 1;
                else
                    value.value = 0;
                break;
            case LARGER:
                if(eval(op1).value > eval(op1->next).value)
                    value.value = 1;
                else
                    value.value = 0;
                break;
            case INVALID_OPER:
                return lambda(p);
            default:
                return value;
        }
    }
    return value;
}

