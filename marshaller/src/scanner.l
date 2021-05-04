whitespace				[\n\t\n ]

id							[a-zA-Z_][a-zA-Z0-9_]*
dec		[0-9]+
hex		[0-9a-fA-F]+H
number		{dec}|{hex}

int_types				"long long"{whitespace}|"long"|"int"|"byte"|"short"
unsigned_int_types	"unsigend "{int_types}
signed_int_types		"signed "{int_types}
stdint_types_u			"uint8_t"|"uint16_t"|"uint32_t"|"uint64_t"
stdint_types_s			"int8_t"|"int16_t"|"int32_t"|"int64_t"

int						{int_types}|{unsigned_int_types}|{signed_int_types}|{stdint_types_u}|{stdint_types_s}

float						"long double"|"double"|"float"

string					 "const "?"char"{whitespace}*"*"

%option noyywrap
%option nodefault
%option yylineno 

%x COMMENT
%%

%{
#include <stdlib.h>
#include <stdbool.h>

#include "y.tab.h"
#include <helper.h>
%}

<*>"/*"		{ BEGIN(COMMENT); }
<COMMENT>"*/"	{ BEGIN(INITIAL); }
<COMMENT>[^*\n]	{ }
<COMMENT>\n	{ }
<COMMENT>"*"[^*\/\n]*	{ }

<INITIAL>"//"[^\n]\n		{ /* single line comment */ }

<INITIAL>^"#"[^\n]*"\n"	{ /* includes, macros, ... */ }

<INITIAL>"typedef"	{ return TYPEDEF; }
<INITIAL>"struct"		{ return STRUCT; }

<INITIAL>{int}		{ return LONG; }
<INITIAL>{float}	{ return DOUBLE; }
<INITIAL>{string}	{ return STRING; }
<INITIAL>"*"		{ return POINTER; }

<INITIAL>";"		{ return SEMICOLON; }
<INITIAL>"{"		{ return OPEN_BRACES; }
<INITIAL>"}"		{ return CLOSE_BRACES; }
<INITIAL>"["		{ return OPEN_BRACKETS; }
<INITIAL>"]"		{ return CLOSE_BRACKETS; }

<INITIAL>{id}		{ yylval.id = strdup(yytext); return ID; }
<INITIAL>{hex}		{ yylval.number = strtol(yytext, NULL, 16); return NUM; }
<INITIAL>{dec}		{ yylval.number = strtol(yytext, NULL, 10); return NUM; }
<INITIAL>{whitespace}+	{ }

<INITIAL>.		{ fprintf(stderr, "lexical error in line %d.\n", yylineno); exit(1); }

