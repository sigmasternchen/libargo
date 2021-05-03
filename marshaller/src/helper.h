#ifndef HELPER_H
#define HELPER_H

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

void _panic(const char* f, const char* s);
#define panic(s) _panic(__func__, s)

#endif
