#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <json.h>

#include "marshaller.h"

void _marshallPanic(const char* name, const char* reason) {
	if (reason == NULL) {
		reason = strerror(errno);
	}

	fprintf(stderr, "panic: marshaller (%s): %s\n", name, reason);
	exit(2);
}

static struct marshaller {
	const char* name;
	jsonValue_t* (*marshaller)(void*);
	void* (*unmarshaller)(jsonValue_t*);
	void (*free)(void*);
}* marshallerList = NULL;
static size_t marshallerListLength = 0;

static struct marshaller* findMarshaller(const char* type) {
	for (size_t i = 0; i < marshallerListLength; i++) {
		if (strcmp(type, marshallerList[i].name) == 0) {
			return &marshallerList[i];
		}
	}
	
	return NULL;
}

void _registerMarshaller(int namesCount, const char** names, jsonValue_t* (*marshaller)(void*),  void* (*unmarshaller)(jsonValue_t*), void (*structFree)(void*)) {
	marshallerList = realloc(marshallerList, (sizeof(struct marshaller)) * (marshallerListLength + namesCount));
	if (marshallerList == NULL) {
		_marshallPanic(names[0], NULL);
	}
	
	for (int i = 0; i < namesCount; i++) {
		if (findMarshaller(names[i])) {
			_marshallPanic(names[i], "marshaller for name already present");
		}
	
		marshallerList[marshallerListLength++] = (struct marshaller) {
			.name = names[i],
			.marshaller = marshaller,
			.unmarshaller = unmarshaller,
			.free = structFree
		};
	}
}

static jsonValue_t* json_marshall_char(void* value) {
	return json_long(*((char*) value));
}

static jsonValue_t* json_marshall_short(void* value) {
	return json_long(*((short*) value));
}

static jsonValue_t* json_marshall_int(void* value) {
	return json_long(*((int*) value));
}

static jsonValue_t* json_marshall_long(void* value) {
	return json_long(*((long*) value));
}

static jsonValue_t* json_marshall_long_long(void* value) {
	return json_long(*((long long*) value));
}

static jsonValue_t* json_marshall_float(void* value) {
	return json_double(*((float*) value));
}

static jsonValue_t* json_marshall_double(void* value) {
	return json_double(*((double*) value));
}

static jsonValue_t* json_marshall_string(void* value) {
	return json_string((const char*) value);
}

static jsonValue_t* json_marshall_bool(void* value) {
	return json_bool(*((bool*) value));
}

jsonValue_t* _json_marshall_value(const char* type, void* value) {
	if (value == NULL) {
		return json_null();
	} else if (strcmp(type, "char") == 0) {
		return json_marshall_char(value);
	} else if (strcmp(type, "short") == 0) {
		return json_marshall_short(value);
	} else if (strcmp(type, "int") == 0) {
		return json_marshall_int(value);
	} else if (strcmp(type, "long") == 0) {
		return json_marshall_long(value);
	} else if (strcmp(type, "long long") == 0) {
		return json_marshall_long_long(value);
	} else if (strcmp(type, "float") == 0) {
		return json_marshall_float(value);
	} else if (strcmp(type, "double") == 0) {
		return json_marshall_double(value);
	} else if (strcmp(type, "string") == 0) {
		return json_marshall_string(value);
	} else if (strcmp(type, "bool") == 0) {
		return json_marshall_bool(value);	
	} else {
		struct marshaller* marshaller = findMarshaller(type);
		if (marshaller == NULL) {
			_marshallPanic(type, "unknown type");
		}
		return marshaller->marshaller(value);
	}
}
char* _json_marshall(const char* type, void* value) {
	return json_stringify(_json_marshall_value(type, value));
}

static void* json_unmarshall_char(jsonValue_t* value) {
	if (value->type != JSON_LONG)
		return NULL;

	char* tmp = malloc(sizeof(char));
	if (tmp == NULL)
		return NULL;
		
	*tmp = value->value.integer;
	return tmp;
}

static void* json_unmarshall_short(jsonValue_t* value) {
	if (value->type != JSON_LONG)
		return NULL;

	short* tmp = malloc(sizeof(short));
	if (tmp == NULL)
		return NULL;
		
	*tmp = value->value.integer;
	return tmp;
}

static void* json_unmarshall_int(jsonValue_t* value) {
	if (value->type != JSON_LONG)
		return NULL;

	int* tmp = malloc(sizeof(int));
	if (tmp == NULL)
		return NULL;
		
	*tmp = value->value.integer;
	return tmp;
}

static void* json_unmarshall_long(jsonValue_t* value) {
	if (value->type != JSON_LONG)
		return NULL;

	long* tmp = malloc(sizeof(long));
	if (tmp == NULL)
		return NULL;
		
	*tmp = value->value.integer;
	return tmp;
}

static void* json_unmarshall_long_long(jsonValue_t* value) {
	if (value->type != JSON_LONG)
		return NULL;

	long long* tmp = malloc(sizeof(long long));
	if (tmp == NULL)
		return NULL;
		
	*tmp = value->value.integer;
	return tmp;
}

static void* json_unmarshall_float(jsonValue_t* value) {
	if (value->type != JSON_DOUBLE && value->type != JSON_LONG)
		return NULL;

	float* tmp = malloc(sizeof(float));
	if (tmp == NULL)
		return NULL;
		
	if (value->type == JSON_DOUBLE)
		*tmp = value->value.real;
	else
		*tmp = value->value.integer;
	return tmp;
}

static void* json_unmarshall_double(jsonValue_t* value) {
	if (value->type != JSON_DOUBLE && value->type != JSON_LONG)
		return NULL;

	double* tmp = malloc(sizeof(double));
	if (tmp == NULL)
		return NULL;
		
	if (value->type == JSON_DOUBLE)
		*tmp = value->value.real;
	else
		*tmp = value->value.integer;
	return tmp;
}

static void* json_unmarshall_bool(jsonValue_t* value) {
	if (value->type != JSON_BOOL)
		return NULL;

	bool* tmp = malloc(sizeof(bool));
	if (tmp == NULL)
		return NULL;
		
	*tmp = value->value.boolean;
	return tmp;
}

static void* json_unmarshall_string(jsonValue_t* value) {
	if (value->type != JSON_STRING)
		return NULL;

	char* tmp = strdup(value->value.string);
	
	return tmp;
}

void* _json_unmarshall_value(const char* type, jsonValue_t* value) {
	if (value->type == JSON_NULL) {
		return NULL;
	} else if (strcmp(type, "char") == 0) {
		return json_unmarshall_char(value);
	} else if (strcmp(type, "short") == 0) {
		return json_unmarshall_short(value);
	} else if (strcmp(type, "int") == 0) {
		return json_unmarshall_int(value);
	} else if (strcmp(type, "long") == 0) {
		return json_unmarshall_long(value);
	} else if (strcmp(type, "long long") == 0) {
		return json_unmarshall_long_long(value);
	} else if (strcmp(type, "float") == 0) {
		return json_unmarshall_float(value);
	} else if (strcmp(type, "double") == 0) {
		return json_unmarshall_double(value);
	} else if (strcmp(type, "string") == 0) {
		return json_unmarshall_string(value);
	} else if (strcmp(type, "bool") == 0) {
		return json_unmarshall_bool(value);	
	} else {
		struct marshaller* marshaller = findMarshaller(type);
		if (marshaller == NULL) {
			_marshallPanic(type, "unknown type");
		}
		return marshaller->unmarshaller(value);
	}
}

void* _json_unmarshall(const char* type, const char* json) {
	jsonValue_t* value = json_parse(json);
	if (value == NULL) {
		return NULL;
	}
	
	void* tmp = _json_unmarshall_value(type, value);
	free(value);
	return tmp;
}

void _json_free_struct(const char* type, void* value) {
	if (value == NULL) {
		return;
	} else if (strcmp(type, "char") == 0 ||
              strcmp(type, "short") == 0 ||
              strcmp(type, "int") == 0 ||
              strcmp(type, "long") == 0 ||
              strcmp(type, "long long") == 0 ||
              strcmp(type, "float") == 0 ||
              strcmp(type, "double") == 0 ||
              strcmp(type, "bool") == 0 ||
              strcmp(type, "string") == 0
	) {
		free(value);
	} else {
		struct marshaller* marshaller = findMarshaller(type);
		if (marshaller == NULL) {
			_marshallPanic(type, "unknown type");
		}
		marshaller->free(value);
	}
}
