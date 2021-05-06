
%{
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>

#include <helper.h>
#include <codegen.h>

int yylex();
void yyerror(const char*, char*);

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
%token CHAR SHORT INT LONG LONG_LONG FLOAT DOUBLE STRING CONST_STRING STDINT
%token POINTER
%token SEMICOLON OPEN_BRACES CLOSE_BRACES OPEN_BRACKETS CLOSE_BRACKETS
%token <id> ID
%token NUM

%parse-param {const char* filename}

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
     | declars declar SEMICOLON 
     			{
     				$$ = $1;
     				addStruct($$, $2);
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
             | structmembers structmember SEMICOLON
				{
					$$ = $1;
					addMember($$, $2);
				}
;

structmember: type ID
				{
					$$ = newMemberInfo($1, $2);
				}
				| type ID OPEN_BRACKETS NUM CLOSE_BRACKETS
				{
					yyerror(filename, ERROR("static array members not yet supported"));
					YYERROR;
				}
				| type ID OPEN_BRACKETS CLOSE_BRACKETS
				{
					yyerror(filename, ERROR("flexible array members not supported; use double pointers for dynamic array instead"));
					YYERROR;
				}
;

type: STDINT		{ yyerror(filename, ERROR("stdint types are not yet supported")); YYERROR; }
    | CHAR			{ $$ = newTypeInfo(false, "char"); }
    | SHORT			{ $$ = newTypeInfo(false, "short"); }
    | INT			{ $$ = newTypeInfo(false, "int"); }
    | LONG			{ $$ = newTypeInfo(false, "long"); }
    | LONG_LONG	{ $$ = newTypeInfo(false, "long long"); }
    | FLOAT			{ $$ = newTypeInfo(false, "float"); }
    | DOUBLE		{ $$ = newTypeInfo(false, "double"); }
    | STRING		{ $$ = newTypeInfo(false, "string"); }
    | CONST_STRING { $$ = newTypeInfo(false, "string"); yyerror(filename, WARN("const char* struct members are discouraged")); }
    | STRUCT ID	{ $$ = newTypeInfo(false, prefix($2, "struct ")); }
    | ID				{ $$ = newTypeInfo(false, $1); }
    | type POINTER
				{
					if ($1->isPointer || strcmp($1->type, "string") == 0) {
						if ($1->isArray) {
							yyerror(filename, ERROR("multi pointer types are not supported"));
							YYERROR;
						} else {
							yyerror(filename, INFO("double pointer type; assuming dynamic array"));
							$$ = $1;
							$$->isArray = true;
						}
					} else {
						$$ = newTypeInfo(true, $1->type);
					}
				}
;

%%

void yyerror(const char* filename, char* s) {
	extern int yylineno;
	fprintf(stderr, "%s (%s, line %d)\n", s, filename, yylineno);
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
	
	return info;
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
