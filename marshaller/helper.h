#ifndef HELPER_H
#define HELPER_H

void _panic(const char* f, const char* s);
#define panic(s) _panic(__func__, s)

#define WARN(msg) ("\033[33mwarning:\033[0m " msg)
#define ERROR(msg) ("\033[31mwerror:\033[0m " msg)

#endif
