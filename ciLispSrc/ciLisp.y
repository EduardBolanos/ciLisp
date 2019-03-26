/*
    Eduard Bolanos
    Completed ciLisp without any errors, using test cases provided to check everything works properly.
*/

%{
    #include "ciLisp.h"
%}

%union {
    double dval;
    char *sval;
    struct ast_node *astNode;
    struct symbol_table_node *symNode;
};

%token <sval> FUNC
%token <sval> SYMBOL
%token <dval> NUMBER
%token <sval> TYPE
%token LET LPAREN RPAREN EOL QUIT COND LAMBDA

%type <astNode> s_expr s_expr_list
%type <symNode> let_elem let_list scope arg_list arg_elem

%%

program:
    s_expr EOL {
        fprintf(stderr, "yacc: program ::= s_expr EOL\n");
        if ($1) {
            printf("%lf", eval($1).value);

            freeNode($1);
        }
    };

s_expr:
    NUMBER {
        fprintf(stderr, "yacc: s_expr ::= NUMBER\n");
        $$ = number($1);
    }
    | LPAREN FUNC s_expr_list RPAREN {
       fprintf(stderr, "yacc: LPAREN FUNC expr RPAREN\n");
        $$ = function($2,$3);
    }
    | LPAREN SYMBOL s_expr_list RPAREN {
        fprintf(stderr, "yacc: LPAREN FUNC expr RPAREN\n");
        $$ = function($2,$3);
    }
    | QUIT {
        fprintf(stderr, "yacc: s_expr ::= QUIT\n");
        exit(EXIT_SUCCESS);
    }
    | error {
        fprintf(stderr, "yacc: s_expr ::= error\n");
        yyerror("unexpected token");
        $$ = NULL;
    }
    | SYMBOL {
        $$ = symbol($1);
    }
    | LPAREN scope s_expr RPAREN {
        fprintf(stderr, "yacc: ");
        $$ = setScope($2, $3);
    };
    | LPAREN COND s_expr s_expr s_expr RPAREN
    {
        fprintf(stderr, "yacc: LPAREN COND expr expr expr RPAREN\n");
        $$ = condition($3, $4, $5);

    };


s_expr_list:
    /* empty */{
        $$ = NULL;
    }
    | s_expr {
        $$ = $1;
    }
    | s_expr_list s_expr {
        $$ = expr_list($1, $2);
    };

scope:
    /* empty */{
            $$ = NULL;
    }
    | LPAREN LET let_list RPAREN {
            //fprintf(stderr, "yacc: s_expr ::= LPAREN LET let_list RPAREN\n");
         $$ = $3;
    };

let_list:
    let_elem {
       $$ = $1;
    }
    | let_list let_elem {
        $$ = let_list($1, $2);
    };

let_elem:
    LPAREN TYPE SYMBOL s_expr RPAREN {
        $$ = let_elem($2, $3, NULL, $4);
    }
    | LPAREN SYMBOL s_expr RPAREN {
        $$ = let_elem(NULL, $2, NULL, $3);
    }
    | LPAREN TYPE SYMBOL LAMBDA LPAREN arg_list RPAREN s_expr RPAREN {
        $$ = let_elem($2, $3, $6, $8);
    }| LPAREN SYMBOL LAMBDA LPAREN arg_list RPAREN s_expr RPAREN {
        $$ = let_elem(NULL, $2, $5, $7);
    };

arg_elem:
    SYMBOL {
        $$ = arg_elem($1);
    };

arg_list:
    arg_list arg_elem {
        $$ = arg_list($1, $2);
    }
    | arg_elem {
        $$ = $1;
    };


%%

