#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "codegen.h"
#include "helper.h"

struct declarsinfo* declarations;

extern int yyparse();

void generatePreamble(FILE* output) {
	fprintf(output, "#include <stdlib.h>\n");
	fprintf(output, "#include <stdbool.h>\n");
	fprintf(output, "#include <errno.h>\n");
	fprintf(output, "#include <alloc.h>\n");
	fprintf(output, "\n");
	fprintf(output, "#include <json.h>\n");
	fprintf(output, "#include <marshaller.h>\n");
	fprintf(output, "\n");
	fprintf(output, "extern void _marshallPanic(const char*, const char*);\n");
	fprintf(output, "extern void _registerMarshaller(int, const char**, (jsonValue_t*)(*)(void*), (void*)(*)(jsonValue_t*));\n");
	fprintf(output, "\n");
}

char* fixStructName(char* _name) {
	char* name = strdup(_name);
	if (name == NULL) {
		panic("strdup");
	}
	
	for (size_t i = 0; name[i] != '\0'; i++) {
		if (name[i] == ' ') {
			name[i] = '_';
		}
	}
	
	return name;
}

char* generateMarshallFunction(FILE* output, struct structinfo* info, char* suffix) {
	#define MARSHALL_FUNCTION_PREFIX "_json_marshall_value_"

	char* functionName = malloc(strlen(MARSHALL_FUNCTION_PREFIX) + strlen(suffix) + 1);
	if (functionName == NULL) {
		panic("malloc");
	}
	strcpy(functionName, MARSHALL_FUNCTION_PREFIX);
	strcat(functionName, suffix);
	
	fprintf(output, "static jsonValue_t* %s(%s* d) {\n", functionName, info->names[0]);
	fprintf(output, "\tif (d == NULL)\n");
	fprintf(output, "\t\treturn json_null();\n");
	fprintf(output, "\tretuen json_object(true, %d\n", info->memberno);

	for (size_t i = 0; i < info->memberno; i++) {
		struct memberinfo* member = info->members[i];
		
		fprintf(output, "\t\t\"%s\", ", member->name);
		
		const char* reference = "";
		
		if (!member->type->isPointer) {
			reference = "&";
		}
		
		fprintf(output, "_json_marshall_value(\"%s\", %s(d->%s))%s\n", member->type->type, reference, member->name, i == info->memberno - 1 ? "," : "");
	}
	
	fprintf(output, "\t);\n");
	fprintf(output, "}\n\n");
	
	return functionName;
}

char* generateUnmarshallFunction(FILE* output, struct structinfo* info, char* suffix) {
	#define UNMARSHALL_FUNCTION_PREFIX "_json_unmarshall_value_"

	char* functionName = malloc(strlen(UNMARSHALL_FUNCTION_PREFIX) + strlen(suffix) + 1);
	if (functionName == NULL) {
		panic("malloc");
	}
	strcpy(functionName, UNMARSHALL_FUNCTION_PREFIX);
	strcat(functionName, suffix);
	
	fprintf(output, "static %s* %s(jsonValue_t* v) {\n", info->names[0], functionName);
	fprintf(output, "\tif (v->type != JSON_OBJECT) {\n");
	fprintf(output, "\t\terrno = EINVAL;\n");
	fprintf(output, "\t\treturn NULL;\n");
	fprintf(output, "\t}\n");
	fprintf(output, "\t%s* d = malloc(sizeof(%s));\n", info->names[0], info->names[0]);
	fprintf(output, "\tif (d == NULL)\n");
	fprintf(output, "\t\treturn NULL;\n");
	fprintf(output, "\tvoid* tmp;\n");
	
	
	for (size_t i = 0; i < info->memberno; i++) {
		struct memberinfo* member = info->members[i];
		fprintf(output, "\ttmp = _json_unmarshall_value(\"%s\", json_object_get(v, \"%s\");\n", member->type->type, member->name);
		if (member->type->isPointer) {
			fprintf(output, "\td->%s = (%s*) tmp;\n", member->name, member->type->type);
		} else {
			fprintf(output, "\tif (tmp == NULL) {\n");
			fprintf(output, "\t\terrno = EINVAL;\n");
			fprintf(output, "\t\tfree(d);\n");
			fprintf(output, "\t\treturn NULL;\n");
			fprintf(output, "\t} else {\n");
			fprintf(output, "\t\td->%s = *((%s*) tmp);\n", member->name, member->type->type);
			fprintf(output, "\t\tfree(tmp);\n");
			fprintf(output, "\t}\n");
		}
	}
	
	fprintf(output, "\treturn d;\n");
	
	fprintf(output, "}\n\n");
	
	return functionName;
}

bool isDefaultType

void generateCodeStruct(FILE* output, struct structinfo* info) {
	char* suffix = fixStructName(info->names[0]);
	
	char* marshall = generateMarshallFunction(output, info, suffix);
	char* unmarshall = generateUnmarshallFunction(output, info, suffix);
	
	fprintf(output, "__attribute__((constructor)) static void _register_marshaller_%s_() {\n", suffix);
	int namesno = info->names[1] == 0 ? 1 : 2;
	fprintf(output, "\tchar* tmp = alloca(sizeof(char*) * %d);\n", namesno);
	fprintf(output, "\ttmp[0] = \"%s\";\n", info->names[0]);
	fprintf(output, "\ttmp[1] = \"%s\";\n", info->names[1]);
	fprintf(output, "\t_registerMarshaller(%d, tmp, &%s, &%s);\n", namesno, marshall, unmarshall);
	fprintf(output, "}\n\n");
	
	free(suffix);
	
	free(marshall);
	free(unmarshall);
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
	
	generatePreamble(stdout);
	generateCode(stdout);
	
	return 0;
}
