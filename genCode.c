/*
Some assumptions were made during the development of this program.
1. Since a syntax error is thrown if something is syntactically incorrect, I assumed (for rules #2,3) that the presence of the incorrect operator in either meant that it was wrong. Or if it lacks its corresponding operator. For example, if we have an int variable X, and we try to assign it some value with an OP4 operator present, it must be wrong. If we try to assign a bool variable Y without at least one OP4 operator present, it must be wrong. Also, for the if and while loops, I took it as if there is at least one OP4 operator, or there is only a boolean variable/constant, then it is valid. Otherwise, an error will show.
2. The control structures and most of the operators seemed to be exactly like the C-family, Java, etc., so I coded them like I would in C. For example, 
if SMALLER > BIGGER then
      TEMP := SMALLER ;
      TEMP1 := 2147483648 ;
      SMALLER := BIGGER ;
      BIGGER := TEMP ;
   end ;
gets coded (in C) as:
if(SMALLER > BIGGER)
{
    TEMP = SMALLER;
    TEMP1 = 2147483648;
    SMALLER = BIGGER;
    BIGGER = TEMP;
}
3. -2147483647 through 2147483647 is assumed to be the range of valid numbers.
4. "All variables must be declared with a particular type." I took that every variable must have either bool or int in both the generated C code and TL13. The grammar doesn't allow the absence of a variable type, and because of that, there shouldn't be a case where there isn't one in the generated C code. Unless, of course, it wasn't declared.
*/

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "structs.h"
#include "test.tab.h"
#include "Downloads/uthash-master/include/uthash.h"

void genCodeProgram(struct Prgm * p);
void genCodeDecls(struct Dclrns * d);
void genCodeType(int option);
void genCodeStmtSeq(struct Stmt_seq *seq);
void genCodeAssignment(struct Asgnmt * a);
void genCodeIf(struct If_stmt * i);
void genCodeElse(struct Else_cls * e);
void genCodeWhile(struct While_stmt * w);
void genCodeWriteInt(struct W_int * t);

bool genCodeExpr(struct Expr * f, int opt);
void genCodeSimpExp(struct Smpl_expr * s, int opt);
void genCodeTerm(struct Trm * t, int opt);
void genCodeFactor(struct Fact * f, int opt);

//functions to call from inside this file
void genCodeStmt2(struct Stmt * s);
void genCodeExpr2(struct Expr * e, int opt, int opt2);
void genCodeSimpExp2(struct Smpl_expr * s, int opt, int opt2);
void genCodeTerm2(struct Trm * t, int opt, int opt2);
void genCodeFactor2(struct Fact * f, int opt, int opt2);
bool checkOperandTypeTerm(struct Trm * t);
bool checkOperandTypeSimpExp(struct Smpl_expr *s);
bool checkOperandTypeExpr(struct Expr * e);
void iterate();
char * genCodeExpr3(struct Expr * e);
char * genCodeSimpExp3(struct Smpl_expr * s);
char * genCodeTerm3(struct Trm * t);
char * genCodeFactor3(struct Fact * f);
void genCodeExpr4(struct Expr * e);
void genCodeSimpExp4(struct Smpl_expr * s);
void genCodeTerm4(struct Trm * t);
void genCodeFactor4(struct Fact * f);
void genCodeForForcingInt();
struct ST_Entry * findEntry(char * id);

bool hasIntForcingBeenDeclared = false;
static char fHelper[100];

//const char * id is the key used to index the struct
//int type is a numeric representation (1,2) to indicate if it's an int or bool
//bool declared is self-explanatory
//UT_hash_handle is required to use UTHash
struct ST_Entry{
    const char * id;
    int type;
    bool declared;
    UT_hash_handle hh;
};

struct ST_Entry * entries = NULL;

void addEntry(char * id,int type,bool dec)
{
    struct ST_Entry * e = findEntry(id);
    //if the entry DOESN'T exist in the symbol table
    if(e==NULL)
    {
        e = malloc(sizeof *e);
        e->id = id;
        e->type = type;
        e->declared = dec;
        HASH_ADD_KEYPTR(hh,entries,e->id,strlen(e->id),e);
    }
    //if the entry DOES exist in the symbol table
    else
    {
        //error: #5
        printf("ERROR: Redeclaration of variable %s\n",id);
    }
}

struct ST_Entry * findEntry(char * id)
{
    struct ST_Entry * e;
    HASH_FIND_STR(entries,id,e);
    return e;
}

void iterate()
{
    struct ST_Entry * e;
    for(e=entries; e!=NULL; e=e->hh.next){
        printf("\nid: %s, type: %s, declared: %s\n",e->id,e->type==1?"int":"bool",e->declared?"true":"false");
    }
}

void genCodeProgram(struct Prgm * p)
{
    printf("#include <stdio.h>\n#include <stdbool.h>\n#include <stdlib.h>\n#include <string.h>\n#include <limits.h>\n\n");
    genCodeDecls(p->d);
    printf("\nint main()\n{\n");
    genCodeStmtSeq(p->seq);
    printf("return 0;\n}");
}

void genCodeDecls(struct Dclrns * d)
{
    if(!d->empty)
    {    
        if(!d->t->which)
        {
            genCodeType(1);
            printf("%s = 0;\n",d->id);
            addEntry(d->id,1,true);
        }
        else
        {
            genCodeType(2);
            printf("%s = false;\n",d->id);
            addEntry(d->id,2,true);
        }
        genCodeDecls(d->d);
    }
}

void genCodeType(int option)
{
    if (option==1)
        printf("int ");
    else
        printf("bool ");     
}

void genCodeStmtSeq(struct Stmt_seq *seq)
{
    if(!seq->empty)
    {
        genCodeStmt2(seq->s);
        printf("\n");
        genCodeStmtSeq(seq->seq);
    }
}

void genCodeForForcingInt(char * id)
{
    // code from https://stackoverflow.com/questions/26583717 how-to-scanf-only-integer
    // char *end;
    // char buf[12];
    // int n = 0;
    // do {
    // if (!fgets(buf, sizeof buf, stdin))
    // break;
    // // remove \n
    // buf[strcspn(buf, "\n")] = '\0';
    // n = strtol(buf, &end, 10);
    // } while (end != buf + strlen(buf));
    if(!hasIntForcingBeenDeclared)
    {
        printf("char * end;\nchar buf[12];\nint varForForcingInt=0;\ndo{\nif (!fgets(buf, sizeof buf, stdin)) break;\nbuf[strcspn(buf, \"\\n\")] = '\\0';\nvarForForcingInt=strtol(buf, &end, 10);\n} while (end != buf + strlen(buf)); \n%s=varForForcingInt;\n",id);
        hasIntForcingBeenDeclared=true;
    }
    else
    {
        printf("do{\nif (!fgets(buf, sizeof buf, stdin)) break;\nbuf[strcspn(buf, \"\\n\")] = '\\0';\nvarForForcingInt=strtol(buf, &end, 10);\n} while (end != buf + strlen(buf));\n%s=varForForcingInt;\n",id);
    }
}

void genCodeAssignment(struct Asgnmt * a)
{
    struct ST_Entry * s = findEntry(a->id);
    //if a variable has been declared and is in the symbol table
    if(s)
    {
        if(s->type==1)
        {
            if(a->e) //expression field is valid
            {
                //error: #14
                genCodeExpr4(a->e);
                printf("%s = ",a->id);
                genCodeExpr2(a->e,1,1);
                printf(";");
                //error: #2
                if(!genCodeExpr(a->e,1))
                    printf("\nWARNING: Attempt to assign boolean value to integer.\n");
            }
            else
                genCodeForForcingInt(a->id);
        }
        else
        {
            bool valid = false;
            if(a->e) //expression field is valid
            {
                printf("%s = ",a->id);
                genCodeExpr2(a->e,1,1);
                printf(";");
                char * tm;
                genCodeExpr2(a->e,2,1);
                tm=malloc(strlen(fHelper)+1);
                strcpy(tm,fHelper);
                printf("%s\n",tm);
                bool valid = false;
                if(strcmp(tm,"true")==0 || strcmp(tm,"false")==0 )
                    valid = true;
                if(!valid){
                struct ST_Entry * e = findEntry(tm);
                if(e!=NULL)
                    if(e->type==2 && e->declared)
                        valid = true;    
                //error: #2
                else if(!genCodeExpr(a->e,2))
                    valid = true;
                }
                if(!valid){
                    printf("\nWARNING: Invalid assignment to boolean variable.\n");
                }
            }
            else
            {
                //error: #8
                printf("ERROR: Attempt to assign value of readInt/scanf() to a bool variable!");
            }
        }
    }   //error: #2
    //undeclared variable is trying to be assigned a value 
    else
    {
        //error: #13
        printf("ERROR: %s has not been declared!\n",a->id);
        if(a->e) //expression field is valid
        {
            //error: #14
            genCodeExpr4(a->e);
            printf("%s = ",a->id);
            genCodeExpr2(a->e,1,1);
            printf(";");
            //error: #2
            if(!genCodeExpr(a->e,1))
                printf("\nWARNING: Attempt to assign boolean value to integer.\n");
        }
        else
        {
            genCodeForForcingInt(a->id);
        }
        addEntry(a->id,1,false);
    }
}   

void genCodeIf(struct If_stmt * i)
{
    //error: #10
    char * tm;
    memset(fHelper,0,100);
    genCodeExpr2(i->e,2,0);
    tm=malloc(strlen(fHelper)+1);
    strcpy(tm,fHelper);
    bool valid = false;
    for(int i = 0; i < strlen(tm); i++)
    {
        if(tm[i]=='<' ||tm[i]=='>' ||tm[i]=='!=' ||tm[i]=='==' ||tm[i]=='>=' ||tm[i]=='<='){
            valid = true;
            break;
        }
    }
    if(!valid){
        if(strcmp(tm,"true")==0 || strcmp(tm,"false")==0 )
            valid = true;
        struct ST_Entry * e = findEntry(tm);
        if(e!=NULL)
            if(e->type==2 && e->declared)
                valid = true;    
    }
    if(!valid)
        printf("ERROR: The if statement condition is not boolean.\n");
    printf("if (");
    if(valid)
        genCodeExpr2(i->e,1,0);
    else
        genCodeExpr2(i->e,1,1);
    printf(")\n{\n");
    genCodeStmtSeq(i->seq);
    genCodeElse(i->cls);
    printf("}\n");
}

void genCodeElse(struct Else_cls * e)
{
    if(!e->empty)
    {
        printf("else\n{");
        genCodeStmtSeq(e->seq);
        printf("}");
    }
}

void genCodeWhile(struct While_stmt * w)
{
    //error: #10
    char * tm;
    memset(fHelper,0,100);
    genCodeExpr2(w->e,2,0);
    tm=malloc(strlen(fHelper)+1);
    strcpy(tm,fHelper);
    bool valid = false;
    for(int i = 0; i < strlen(tm); i++)
    {
        if(tm[i]=='<' ||tm[i]=='>' ||tm[i]=='!=' ||tm[i]=='==' ||tm[i]=='>=' ||tm[i]=='<='){
            valid = true;
            break;
        }
    }
    if(!valid){
        if(strcmp(tm,"true")==0 || strcmp(tm,"false")==0 )
            valid = true;
        struct ST_Entry * e = findEntry(tm);
        if(e!=NULL)
            if(e->type==2 && e->declared)
                valid = true;    
    }
    if(!valid)
        printf("ERROR: The while statement condition is not boolean.\n");
    printf("while(");
    if(valid)
        genCodeExpr2(w->e,1,0);
    else
        genCodeExpr2(w->e,1,1);
    printf(")\n{\n");
    genCodeStmtSeq(w->seq);
    printf("}");
}

void genCodeWriteInt(struct W_int * t)
{
    printf("printf(\"%%d\",");
    genCodeExpr2(t->arg,1,0);
    printf(");\n");
}

//functions to call from inside this file.
void genCodeStmt2(struct Stmt * s)
{
    if(s->a)
        genCodeAssignment(s->a);
    else if(s->i)
        genCodeIf(s->i);
    else if(s->w)
        genCodeWhile(s->w);
    else if(s->wr)
        genCodeWriteInt(s->wr);
}

void genCodeExpr2(struct Expr * e, int opt, int opt2)
{
   // printf("expr opt: %d\n",opt);
    if(e->single){
        genCodeSimpExp2(e->single,opt,opt2);
    }
    else
    {
        genCodeSimpExp2(e->left,opt,opt2);
        if(strcmp(e->op4,"=")==0){
            if(opt==1)
                printf("==");
            else
                strcat(fHelper,"==");
        }
        else{
            if(opt==1)
                printf("%s", e->op4);
            else
                strcat(fHelper,e->op4);
        }
        genCodeSimpExp2(e->right,opt,opt2);
    }
}

void genCodeSimpExp2(struct Smpl_expr * s, int opt, int opt2)
{
    //printf("sexpr opt: %d\n",opt);
    if(s->single){
        genCodeTerm2(s->single,opt,opt2);
    }
    else
    {
        genCodeTerm2(s->left,opt,opt2);
        if(opt==1)
            printf("%s",s->op3);
        else
            strcat(fHelper,s->op3);
        genCodeTerm2(s->right,opt,opt2);
    }   
}

void genCodeTerm2(struct Trm * t, int opt, int opt2)
{
    //printf("term opt: %d\n",opt);
    char * tm;
    bool found = false;
    if(t->single)
    {
        genCodeFactor2(t->single,opt,opt2);
    }
    else
    {
        genCodeFactor2(t->left,opt,opt2);
        if(strcmp("div",t->op2)==0){
            if(opt==1)
                printf("/");
            else
                strcat(fHelper,"/");
        }
        else if(strcmp("mod",t->op2)==0){
            if(opt==1)
                printf("%%");
            else
                strcat(fHelper,"%");
        }
        else{
            if(opt==1)
                printf("*");
            else
                strcat(fHelper,"*");
        }
        genCodeFactor2(t->right,opt,opt2);
    }
    if(opt==1 && opt2==1){
        //error: #1
        if(!checkOperandTypeTerm(t))
            printf("\nERROR: An operand is not of type integer!\n");
    }
}

void genCodeFactor2(struct Fact * f, int opt, int opt2)
{
    //printf("fact opt: %d\n",opt);
    char n[14];

    switch(f->type)
    {
        case 1:
            if(opt==1)
                printf("%s",f->identr);
            else
                strcat(fHelper,f->identr);
            //error: #12
            if(!findEntry(f->identr)->declared)
                printf("\nERROR: Trying to use variable that hasn't been declared! : %s\n",f->identr);
            break;
        case 2:
            if(opt==1){
                printf("%d", f->numb);
                if(f->numb > INT32_MAX || f->numb < (INT32_MIN+1))
                printf("\nERROR: Number out of valid range!\n");
            }
            else{
            sprintf(n,"%d",f->numb);
            strcat(fHelper,n);
            }
            break;
        case 3:
            if(opt==1)
                printf("%s", f->boollitl==0?"true":"false");
            else
                strcat(fHelper,f->boollitl==0?"true":"false");
            break;
        case 4:
            if(opt==1)
                printf("(");
            else
                strcat(fHelper,"(");
            genCodeExpr2(f->e,opt,opt2);
            if(opt==1)
                printf(")");
            else
                strcat(fHelper,")");
            break;
    }
}

//ensure that all operand types are integer
bool checkOperandTypeExpr(struct Expr * e)
{
    if(e->single)
    {
        if(!checkOperandTypeSimpExp(e->single))
            return false;
    }
    else
    {
        if(!checkOperandTypeSimpExp(e->left))
            return false;
        if(!checkOperandTypeSimpExp(e->right))
            return false;
    }
    return true;
}

bool checkOperandTypeSimpExp(struct Smpl_expr *s)
{
    if(s->single)
    {
        if(!checkOperandTypeTerm(s->single))
            return false;
    }
    else
    {
        if(!checkOperandTypeTerm(s->left))
            return false;
        if(!checkOperandTypeTerm(s->right))
            return false;
    }
    return true;
}

bool checkOperandTypeTerm(struct Trm * t)
{
    //if the single variable has a value
    if(t->single)
    {
        if(t->single->type != 2 && (t->single->type == 1 && findEntry(t->single->identr)->type != 1))
            return false;
    }
    else
    {
        if(t->left->type != 2 && (t->left->type == 1 && findEntry(t->left->identr)->type != 1))
            return false;

        if(t->right->type != 2 && (t->right->type == 1 && findEntry(t->right->identr)->type != 1))
            return false;
    }
    return true;
}

//making sure when assigning an integer, no OP4 operator is found. OR when assigning a bool, an OP4 operator is found
bool genCodeExpr(struct Expr * e, int opt)
{
    if(e->single){
        genCodeSimpExp(e->single, opt);
    }
    else
    {
        if(opt==1)
            return false;
        else
            return false;
    }
    return true;
}

void genCodeSimpExp(struct Smpl_expr * s,int opt)
{
    if(s->single){
        genCodeTerm(s->single,opt);
    }
    else
    {
        genCodeTerm(s->left,opt);
        genCodeTerm(s->right,opt);
    }   
}

void genCodeTerm(struct Trm * t, int opt)
{
    if(t->single)
    {
        genCodeFactor(t->single,opt);
    }
    else
    {
        genCodeFactor(t->left,opt);
        genCodeFactor(t->right,opt);
    }
}

void genCodeFactor(struct Fact * f, int opt)
{
    switch(f->type)
    {
        case 4:
            genCodeExpr(f->e,opt);
    }
}

//getting the string value of expression
char * genCodeExpr3(struct Expr * e)
{
    static char ex[100];
    memset(ex,0,100);
    if(e->single){
        strcat(ex,genCodeSimpExp3(e->single));
    }
    else
    {
        strcat(ex,genCodeSimpExp3(e->left));
        if(strcmp(e->op4,"=")==0)
            strcat(ex,"==");
        else
            strcat(ex,e->op4);
        strcat(ex,genCodeSimpExp3(e->right));
    }
    return ex;
}

char * genCodeSimpExp3(struct Smpl_expr * s)
{
    static char se[100];
    memset(se,0,100);
    if(s->single){
        strcat(se,genCodeTerm3(s->single));
    }
    else
    {
        strcat(se,genCodeTerm3(s->left));
        strcat(se,s->op3);
        strcat(se,genCodeTerm3(s->right));
    }   
    return se;
}

char * genCodeTerm3(struct Trm * t)
{
    static char tm[100];
    memset(tm,0,100);
    if(t->single)
    {
        strcat(tm,genCodeFactor3(t->single));
    }
    else
    {
        strcat(tm,genCodeFactor3(t->left));
        if(strcmp("div",t->op2)==0)
            strcat(tm,"/");
        else if(strcmp("mod",t->op2)==0)
            strcat(tm,"%");
        else
            strcat(tm,"*");
        strcat(tm,genCodeFactor3(t->right));
    }
    return tm;
}

char * genCodeFactor3(struct Fact * f)
{
    static char fa[100];
    memset(fa,0,100);
    char buff[14];
    char n[14];
    switch(f->type)
    {
        case 1:
            strcat(fa,f->identr);
            break;
        case 2:
            sprintf(n,"%d",f->numb);
            strcat(fa,n);
            break;
        case 3:
            strcat(fa,f->boollitl==1?"true":"false");
            break;
        case 4:
            strcat(fHelper,"(");
            strcat(fHelper,genCodeExpr3(f->e));
            strcat(fHelper,")");
            strcat(fa,fHelper);
            return fHelper;
            break;
    }
    return fa;
}

//outputting check for div/mod values
void genCodeExpr4(struct Expr * e)
{
    if(e->single){
        genCodeSimpExp4(e->single);
    }
    else
    {
        genCodeSimpExp4(e->left);
        genCodeSimpExp4(e->right);
    }
}

void genCodeSimpExp4(struct Smpl_expr * s)
{
    if(s->single){
        genCodeTerm4(s->single);
    }
    else
    {
        genCodeTerm4(s->left);
        genCodeTerm4(s->right);
    }   
}

void genCodeTerm4(struct Trm * t)
{
    char * tm;
    bool found = false;
    int numOP2 = 0;
    int enc = 0;
    if(t->single)
    {
        genCodeFactor4(t->single);
    }
    else
    {
        genCodeFactor4(t->left);
        genCodeFactor4(t->right);
        //check the x,y values for div and mod
        {
            genCodeTerm2(t,2,0);
            tm=malloc(strlen(fHelper)+1);
            strcpy(tm,fHelper);
            for(int i = 0; i < strlen(tm); i++)
            {
                if(tm[i]=='%' || tm[i]=='/'){
                    found = true;
                    numOP2++;
                    //break;
                }
            }
            if(found)
            {
                bool next = false;
                //OP2 == div
                if(strcmp("div",t->op2)==0)
                {
                    printf("if(");
                    for(int i = 0; i < strlen(tm); i++)
                    {
                        if(tm[i]=='%' || tm[i]=='/'){
                            enc++;
                            if(numOP2==1 || enc==1){
                                i++;
                                next = true;
                            }
                        }
                        if(next)
                            printf("%c",tm[i]);
                    }
                    printf("==0 || (");
                    next = false;
                    for(int i = 0; i < strlen(tm); i++)
                    {
                        if(tm[i]!='%' && tm[i]!='/')
                            printf("%c",tm[i]);
                        else{
                            break;
                        }
                    }
                    printf("== INT_MIN +1 && ");
                    enc=0;
                    for(int i = 0; i < strlen(tm); i++)
                    {
                        if(tm[i]=='%' || tm[i]=='/'){
                            enc++;
                            if(numOP2==1 || enc==1){
                                i++;
                                next = true;
                            }
                        }
                        if(next)
                            printf("%c",tm[i]);
                    }
                    printf("==-1))\n{\nprintf(\"ERROR: At least one number is not valid\"); \nreturn -1;\n}\n");
                }
                else if(strcmp("mod",t->op2)==0)
                {
                    enc=0;
                    printf("if(!(");
                    for(int i = 0; i < strlen(tm); i++)
                    {
                        if(tm[i]!='%' && tm[i]!='/')
                            printf("%c",tm[i]);
                        else{
                            break;
                        }
                    }
                    printf(">-1 && ");
                    for(int i = 0; i < strlen(tm); i++)
                    {
                        if(tm[i]=='%' || tm[i]=='/'){
                            enc++;
                            if(numOP2==1 || enc==1){
                                i++;
                                next = true;
                            }
                        }
                        if(next)
                                printf("%c",tm[i]);
                    }
                    printf(">-1))\n{\nprintf(\"ERROR: At least one number is not valid\");\nreturn -1;\n}\n");
                }
            }
        }
    }
    memset(fHelper,0,100);
}

void genCodeFactor4(struct Fact * f)
{
    switch(f->type)
    {
        case 4:
            genCodeExpr4(f->e);
            break;
    }
}