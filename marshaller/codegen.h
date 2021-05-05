#ifndef CODEGEN_H
#define CODEGEN_H

#include <stdbool.h>

struct typeinfo {
	char* type;
	bool isPointer;
};

struct memberinfo {
	struct typeinfo* type;
	char* name;
};

struct structinfo {
	char* names[2];
	size_t memberno;
	struct memberinfo** members;
};

struct declarsinfo {
	size_t structno;
	struct structinfo** structs;
};

extern struct declarsinfo* declarations;

#endif
