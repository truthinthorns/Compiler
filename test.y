%{
	#include <stdio.h>
	#include <string.h>
	#include "structs.h"
	void yyerror (char const *s);
	int yylex(void);
	extern int yylineno;
%}

%token num
%token boollit
%token ident
%token LP
%token RP
%token ASGN
%token SC
%token OP2
%token OP3
%token OP4
%token IF
%token THEN
%token ELSE
%token BEGN
%token END
%token WHILE
%token DO
%token PROGRAM
%token VAR
%token AS
%token INT
%token BOOL
%token WRITEINT
%token READINT

%union {
    char * id;
    int number;
    char * boolean;
    char * o2;
    char * o3;
    char * o4;
    struct Prgm * pg;
    struct Dclrns * dcls;
    struct Tpe * tp;
    struct Stmt_seq * ss;
    struct Stmt * st;
    struct Asgnmt *asmt;
    struct If_stmt * is;
    struct Else_cls * ec;
    struct While_stmt * wst;
    struct W_int * wi;
    struct Expr *ex;
    struct Smpl_expr * smex;
    struct Trm *tm;
    struct Fact * fct;
}
%type<id> ident;
%type<number> num;
%type<boolean> boollit;

%%
program:
	PROGRAM declarations BEGN statementSequence END {prgm *ptr = malloc(sizeof(*ptr)); ptr->d=$<dcls>2; ptr->seq=$<ss>4; genCodeProgram(ptr); $<pg>$ = ptr;}
	;
declarations:
	VAR ident AS type SC declarations {dclrns *ptr = malloc(sizeof(*ptr)); ptr->id=$<id>2; ptr->t=$<tp>4; ptr->d=$<dcls>6; ptr->empty = false;  $<dcls>$ = ptr;}
	| /* empty */ {dclrns *ptr = malloc(sizeof(*ptr)); ptr->empty=true;  $<dcls>$ = ptr;}
	;
type:
	INT {tpe * ptr = malloc(sizeof(*ptr)); ptr->which=false;  $<tp>$ = ptr;}
	| BOOL {tpe * ptr = malloc(sizeof(*ptr)); ptr->which=true;  $<tp>$ = ptr;}
	;
statementSequence:
	statement SC statementSequence {stmt_seq * ptr=malloc(sizeof(*ptr)); ptr->s=$<st>1;ptr->seq=$<ss>3; ptr->empty=false;  $<ss>$ = ptr;}
	| /*empty*/ {stmt_seq * ptr=malloc(sizeof(*ptr)); ptr->empty=true;  $<ss>$ = ptr;}
	;
statement:
	assignment   {stmt *ptr = malloc(sizeof(*ptr)); ptr->a = $<asmt>1;  $<st>$ = ptr;}
	| ifStatement {stmt *ptr = malloc(sizeof(*ptr)); ptr->i = $<is>1;  $<st>$ = ptr;}
	| whileStatement {stmt *ptr = malloc(sizeof(*ptr)); ptr->w = $<wst>1; $<st>$ = ptr;}
	| writeInt {stmt *ptr = malloc(sizeof(*ptr)); ptr->wr = $<wi>1;  $<st>$ = ptr;}
	;
assignment:
	ident ASGN expression {asgnmt *ptr=malloc(sizeof(*ptr)); ptr->id=$<id>1; ptr->e=$<ex>3;  $<asmt>$ = ptr;}
	| ident ASGN READINT {asgnmt *ptr=malloc(sizeof(*ptr)); ptr->id=$<id>1;  $<asmt>$ = ptr;}
	;
ifStatement:
	IF expression THEN statementSequence elseClause END {if_stmt * ptr = malloc(sizeof(*ptr)); ptr->e=$<ex>2;ptr->seq=$<ss>4;ptr->cls=$<ec>5;  $<is>$ = ptr;}
	;
elseClause:
	ELSE statementSequence {else_cls * ptr=malloc(sizeof(*ptr)); ptr->seq=$<ss>2;  $<ec>$ = ptr;}
	| /*empty*/ {else_cls * ptr=malloc(sizeof(*ptr)); ptr->empty=true;  $<ec>$ = ptr;}
	;
whileStatement:
	WHILE expression DO statementSequence END {while_stmt * ptr=malloc(sizeof(*ptr)); ptr->e=$<ex>2;ptr->seq=$<ss>4;  $<wst>$ = ptr;}
	;
writeInt:
	WRITEINT expression {w_int * ptr = malloc(sizeof(*ptr));ptr->arg=$<ex>2;  $<wi>$ = ptr;}
	;
expression:
	simpleExpression {expr * ptr = malloc(sizeof(*ptr)); ptr->single=$<smex>1; $<ex>$ = ptr;}
	| simpleExpression OP4 simpleExpression {expr *ptr = malloc(sizeof(*ptr)); ptr->left=$<smex>1;ptr->op4=$<o4>2;ptr->right=$<smex>3;$<ex>$ = ptr;}
	;
simpleExpression:
	term {smpl_expr *ptr = malloc(sizeof(*ptr));ptr->single=$<tm>1;  $<smex>$ = ptr;}
	| term OP3 term {smpl_expr *ptr = malloc(sizeof(*ptr)); ptr->left=$<tm>1;ptr->op3=$<o3>2;ptr->right=$<tm>3; $<smex>$ = ptr;}
	;
term:
	factor {trm * ptr = malloc(sizeof(*ptr)); ptr->single = $<fct>1; $<tm>$ = ptr;}
	| factor OP2 factor {trm * ptr = malloc(sizeof(*ptr)); ptr->left=$<fct>1;ptr->op2=$<o2>2;ptr->right=$<fct>3;$<tm>$ = ptr;}
	;	
factor:
	ident {fact * ptr = malloc(sizeof(*ptr)); ptr->identr = $<id>1; ptr->type=1; $<fct>$ = ptr; }
	| num {fact * ptr = malloc(sizeof(*ptr)); ptr->numb = $<number>1; ptr->type=2; $<fct>$ = ptr; }
	| boollit {fact * ptr = malloc(sizeof(*ptr)); if(strcmp($<boolean>1,"true")==0) ptr->boollitl=true; else ptr->boollitl=false;ptr->type=3;  $<fct>$ = ptr; }
	| LP expression RP {fact * ptr = malloc(sizeof(*ptr)); ptr->e = $<ex>2; ptr->type=4;  $<fct>$ = ptr; }
	;
%%

int main(void)
{
	yyparse();
}

void yyerror (char const *s) 
{
	fprintf (stderr, "%s Around line number: %d\n", s,yylineno);
	
}
extern int yylex(void);
