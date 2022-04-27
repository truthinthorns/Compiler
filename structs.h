#include <stdbool.h>

//the keywords are not part of the struct since it will be obvious
//what keywords need to go there (if any) based on the struct.
//<type> just has a bool to differentiate between int/bool (f=int,t=bool)
//bool variable for those that can be empty to maybe make it easier.
typedef struct Prgm{
    struct Dclrns * d;
    struct Stmt_seq * seq;
}prgm;

typedef struct Dclrns{
    char * id;
    struct Tpe * t;
    struct Dclrns * d;
    bool empty;
}dclrns;

typedef struct Tpe{
    bool which;
}tpe;

typedef struct Stmt_seq{
    struct Stmt * s;
    struct Stmt_seq * seq;
    bool empty;
}stmt_seq;

typedef struct Stmt{
    struct Asgnmt * a;
    struct If_stmt * i;
    struct While_stmt * w;
    struct W_int * wr;
}stmt;

typedef struct Asgnmt{
    char * id;
    struct Expr * e;
}asgnmt;  

typedef struct If_stmt{
    struct Expr * e;
    struct Stmt_seq * seq;
    struct Else_cls * cls;
}if_stmt;

typedef struct Else_cls{
    struct Stmt_seq * seq;
    bool empty;
}else_cls;

typedef struct While_stmt{
    struct Expr * e;
    struct Stmt_seq * seq;
}while_stmt;

typedef struct W_int{
    struct Expr * arg;
}w_int;

typedef struct Expr{
    struct Smpl_expr * single;
    struct Smpl_expr * left;
    char * op4;
    struct Smpl_expr * right;
}expr;

typedef struct Smpl_expr{
    struct Trm * left;
    char * op3;
    struct Trm * right;
    struct Trm * single;
}smpl_expr;

typedef struct Trm{
    struct Fact * left;
    char * op2;
    struct Fact * right;
    struct Fact * single;
}trm;

typedef struct Fact{
    char * identr;
    int numb;
    bool boollitl;
    struct Expr * e;
    int type;
}fact;
