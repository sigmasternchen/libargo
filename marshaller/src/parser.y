
%{
#include <stdio.h>
#include <stdlib.h>

int yylex();
void yyerror(char*);
%}

%token TYPEDEF STRUCT
%token LONG DOUBLE STRING 
%token POINTER
%token SEMICOLON OPEN_BRACES CLOSE_BRACES OPEN_BRACKETS CLOSE_BRACKETS
%token ID NUM

%start file

%%

file: declars
;

declars: /* empty */
     | declar SEMICOLON declars
;

declar: structdef
      | typedef
;

structdef: STRUCT ID structbody
;

typedef: TYPEDEF STRUCT structbody ID
       | TYPEDEF STRUCT ID structbody ID
;

structbody: OPEN_BRACES structmembers CLOSE_BRACES
;

structmembers: /* empty */
             | structmember SEMICOLON structmembers
;

structmember: type ID
;

type: LONG
    | DOUBLE
    | STRING
    | STRUCT ID
    | ID
    | type POINTER
;

%%

int main(void) {
	yyparse();
	return 0;
}

void yyerror(char* s) {
	extern int yylineno;
	fprintf(stderr, "%s (line %d)\n", s, yylineno);
	exit(2);
}



