%{
#include <stdio.h>
#include "test.tab.h"
%}
%%
\%.*$				
[\n]                 		{yylineno++;}
[1-9][0-9]*|0 			{
					yylval.number = atoi(yytext);
					return num;
				}
false|true			{
					yylval.boolean = strdup(yytext);
					return boollit;
				}
[A-Z][A-Z0-9]*			{
					yylval.id = strdup(yytext);
					return ident;
				}
\(				{return LP;}
\)				{return RP;}
:=				{return ASGN;}
;				{return SC;}
\*|div|mod			{
					yylval.o2 = strdup(yytext);
					return OP2;
				}
\+|-				{
					yylval.o3 = strdup(yytext);
					return OP3;
				}
=|!=|<|>|<=|>=			{
					yylval.o4 = strdup(yytext);
					return OP4;
				}
if				{return IF;}
then				{return THEN;}
else				{return ELSE;}
begin				{return BEGN;}
end				{return END;}
while				{return WHILE;}
do				{return DO;}
program			{return PROGRAM;}
var				{return VAR;}
as				{return AS;}
int				{return INT;}
bool				{return BOOL;}
writeInt			{return WRITEINT;}
readInt			{return READINT;}	
%%
