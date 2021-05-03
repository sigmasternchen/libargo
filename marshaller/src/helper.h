#ifndef HELPER_H
#define HELPER_H

void _panic(const char* f, const char* s);
#define panic(s) _panic(__func__, s)

#endif
