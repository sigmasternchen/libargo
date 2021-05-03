
%{
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>

#include <helper.h>
#include <codegen.h>

int yylex();
void yyerror(char*);

struct typeinfo* newTypeInfo(bool isPointer, char* type);
struct memberinfo* newMemberInfo(struct typeinfo* type, char* name);
struct structinfo* newStructInfo();
struct declarsinfo* newDeclarsInfo();
void addMember(struct structinfo* _struct, struct memberinfo* member);
void addStruct(struct declarsinfo* declars, struct structinfo* _struct);

char* prefix(char* string, const char* prefix);

%}

%union {
	struct typeinfo* typeinfo;
	struct memberinfo* memberinfo;
	struct structinfo* structinfo;
	struct declarsinfo* declarsinfo;
	long long number;
	char* id;
}

%type <typeinfo> type
%type <memberinfo> structmember
%type <structinfo> structmembers
%type <structinfo> structbody
%type <structinfo> typedef
%type <structinfo> structdef
%type <structinfo> declar
%type <declarsinfo> declars
%type <declarsinfo> file

%token TYPEDEF STRUCT
%token LONG DOUBLE STRING 
%token POINTER
%token SEMICOLON OPEN_BRACES CLOSE_BRACES OPEN_BRACKETS CLOSE_BRACKETS
%token <id> ID
%token NUM

%start file

%%

file: declars
				{
					declarations = $1;
				}
;

declars: /* empty */
				{
					$$ = newDeclarsInfo();
				}
     | declar SEMICOLON declars
     			{
     				$$ = $3;
     				addStruct($3, $1);
     			}
;

declar: structdef
      | typedef
;

structdef: STRUCT ID structbody
				{
					$$ = $3;
					$$->names[0] = prefix($2, "struct ");
				}
;

typedef: TYPEDEF STRUCT structbody ID
				{
					$$ = $3;
					$$->names[0] = $4;
				}
       | TYPEDEF STRUCT ID structbody ID
				{
					$$ = $4;
					$$->names[0] = prefix($3, "struct ");
					$$->names[1] = $5;
				}
;

structbody: OPEN_BRACES structmembers CLOSE_BRACES
				{
					$$ = $2;
				}
;

structmembers: /* empty */
				{
					$$ = newStructInfo();
				}
             | structmember SEMICOLON structmembers
				{
					$$ = $3;
					addMember($$, $1);
				}
;

structmember: type ID
				{
					$$ = newMemberInfo($1, $2);
				}
;

type: LONG			{ $$ = newTypeInfo(false, "long"); }
    | DOUBLE		{ $$ = newTypeInfo(false, "double"); }
    | STRING		{ $$ = newTypeInfo(false, "string"); }
    | STRUCT ID	{ yyerror("structs without typedef are not yet supported"); YYERROR; }
    | ID				{ $$ = newTypeInfo(false, $1); }
    | type POINTER
				{
					if ($1->isPointer || $1->type == "string") {
						yyerror("multiple pointer types are not supported");
						YYERROR;
					} else {
						$$ = newTypeInfo(true, $1->type);
					}
				}
;

%%

void yyerror(char* s) {
	extern int yylineno;
	fprintf(stderr, "%s (line %d)\n", s, yylineno);
	exit(2);
}

struct typeinfo* newTypeInfo(bool isPointer, char* type) {
	struct typeinfo* info = malloc(sizeof(struct typeinfo));
	if (info == NULL) {
		panic("malloc");
	}	
	
	info->isPointer = isPointer;
	
	errno = EINVAL;
	if (type == NULL) {
		panic("type name is NULL");
	}
	info->type = type;
	
	return info;
}

struct memberinfo* newMemberInfo(struct typeinfo* type, char* name) {
	struct memberinfo* info = malloc(sizeof(struct memberinfo));
	if (info == NULL) {
		panic("malloc");
	}
	
	errno = EINVAL;
	if (name == NULL) {
		panic("name is NULL");
	}
	
	info->type = type;
	info->name = name;
	
	return info;
}

struct structinfo* newStructInfo() {
	struct structinfo* info = malloc(sizeof(struct structinfo));
	if (info == NULL) {
		panic("malloc");
	}
	
	info->names[0] = NULL;
	info->names[1] = NULL;
	info->memberno = 0;
	info->members = NULL;
}

struct declarsinfo* newDeclarsInfo() {
	struct declarsinfo* info = malloc(sizeof(struct declarsinfo));
	if (info == NULL) {
		panic("malloc");
	}
	
	info->structno = 0;
	info->structs = NULL;
	
	return info;
}

void addMember(struct structinfo* _struct, struct memberinfo* member) {
	_struct->members = realloc(_struct->members, sizeof(struct memberinfo*) * (_struct->memberno + 1));
	if (_struct->members == NULL) {
		panic("realloc");
	}
	
	_struct->members[_struct->memberno++] = member;
}

void addStruct(struct declarsinfo* declars, struct structinfo* _struct) {
	declars->structs = realloc(declars->structs, sizeof(struct structinfo*) * (declars->structno + 1));
	if (declars->structs == NULL) {
		panic("realloc");
	}
	
	declars->structs[declars->structno++] = _struct;
}

char* prefix(char* string, const char* prefix) {
	char* new = malloc(strlen(string) + strlen(prefix) + 1);
	if (new == NULL) {
		panic("malloc");
	}
	
	strcpy(new, prefix);
	strcat(new, string);
	
	free(string);
	
	return new;
}
