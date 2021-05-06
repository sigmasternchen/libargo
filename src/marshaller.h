#ifndef MARSHALLER_H
#define MARSHALLER_H

#include <stdbool.h>

jsonValue_t* _json_marshall_value(const char* type, void* value);
jsonValue_t* _json_marshall_array_value(const char* type, void* value);
char* _json_marshall(const char* type, void* value);
char* _json_marshall_array(const char* type, void* value);

void* _json_unmarshall_value(const char* type, jsonValue_t* value);
void* _json_unmarshall_array_value(const char* type, jsonValue_t* value);
void* _json_unmarshall(const char* type, const char* json);
void* _json_unmarshall_array(const char* type, const char* json);

void _json_free_struct(const char* type, void* value, bool this);

#define json_marshall(t, v) _json_marshall(# t, (void*) v)
#define json_marshall_array(t, v) _json_marshall_array(# t, (void*) v)
#define json_unmarshall(t, j) (t*) _json_unmarshall(#t, j)
#define json_unmarshall_array(t, j) (t*) _json_unmarshall_array(#t, j)

#define json_free_struct(t, v) _json_free_struct(#t, v, true)

#endif
