whitespace				[\n\t\n ]

id							[a-zA-Z_][a-zA-Z0-9_]*
dec		[0-9]+
hex		[0-9a-fA-F]+H
number		{dec}|{hex}

stdint_types_u			"uint8_t"|"uint16_t"|"uint32_t"|"uint64_t"
stdint_types_s			"int8_t"|"int16_t"|"int32_t"|"int64_t"
stdint_types			{stdint_types_u}|{stdint_types_s}

char						"char"
short						"short"({whitespace}"int")?
int						"int"
long						"long"({whitespace}"int")?
long_long				"long long"{whitespace}"int"?
float						"float"
double					"double"

string					"char"{whitespace}*"*"
const_string			"const"{whitespace}*{string}

%option noyywrap
%option nodefault
%option nounput
%option noinput
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

<INITIAL>{stdint_types}	{ return STDINT; }
<INITIAL>{char}		{ return CHAR; }
<INITIAL>{short}		{ return SHORT; }
<INITIAL>{int}			{ return INT; }
<INITIAL>{long}		{ return LONG; }
<INITIAL>{long_long}	{ return LONG_LONG; }
<INITIAL>{float}		{ return FLOAT; }
<INITIAL>{double}		{ return DOUBLE; }
<INITIAL>{string}		{ return STRING; }
<INITIAL>{const_string}		{ return CONST_STRING; }
<INITIAL>"*"			{ return POINTER; }

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

