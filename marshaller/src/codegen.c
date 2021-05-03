#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "codegen.h"

struct declarsinfo* declarations;

extern int yyparse();

void generateCodeStruct(FILE* output, struct structinfo* info) {
	fprintf(output, "Struct %s", info->names[0]);
	if (info->names[1]) {
		fprintf(output, ", %s", info->names[1]);
	}
	fprintf(output, "\n");
	
	for (size_t i = 0; i < info->memberno; i++) {
		fprintf(output, "   Member %s (%s%s)\n", 
			info->members[i]->name,
			info->members[i]->type->type,
			info->members[i]->type->isPointer ? " pointer" : ""
		);
	}
}

void generateCode(FILE* output) {
	for (size_t i = 0; i < declarations->structno; i++) {
		generateCodeStruct(output, declarations->structs[i]);
	}
}

int main(void) {
	int result = yyparse();
	if (result != 0) {
		return result;
	}
	
	generateCode(stdout);
	
	return 0;
}
