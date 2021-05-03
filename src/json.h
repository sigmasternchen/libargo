#ifndef JSON_H
#define JSON_H

#include <stdlib.h>
#include <stdbool.h>

typedef enum {
	JSON_ARRAY,
	JSON_OBJECT,
	JSON_DOUBLE,
	JSON_LONG,
	JSON_STRING,
	JSON_BOOL,
	JSON_NULL,
} jsonValueType_t;

typedef struct {
	size_t size;
	struct jsonObjectEntry* entries;
} jsonObject_t;

typedef struct {
	size_t size;
	struct jsonValue* entries;
} jsonArray_t;

typedef struct jsonValue {
	jsonValueType_t type;
	union {
		bool boolean;
		double real;
		long long integer;
		char* string;
		jsonObject_t object;
		jsonArray_t array;
	} value;
} jsonValue_t;

typedef struct jsonObjectEntry {
	char* key;
	struct jsonValue value;
} jsonObjectEntry_t;

void json_free(jsonValue_t* value);
jsonValue_t* json_value();

jsonValue_t* json_null();
jsonValue_t* json_double(double d);
jsonValue_t* json_long(long l);
jsonValue_t* json_bool(bool b);
jsonValue_t* json_string(const char* s);
jsonValue_t* json_array(bool freeAfterwards, size_t size, ...);
jsonValue_t* json_object(bool freeAfterwards, size_t size, ...);

void json_print(jsonValue_t* value);

jsonValue_t* json_clone(jsonValue_t* value);

jsonValue_t* json_object_get(jsonValue_t* value, const char* key);
jsonValue_t* json_array_get(jsonValue_t* value, size_t i);
jsonValue_t* json_query(jsonValue_t* value, const char* query);

char* json_stringify(jsonValue_t* value);
jsonValue_t* json_parse(const char* string);

#endif
