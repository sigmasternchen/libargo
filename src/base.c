#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>

#include "json.h"

void json_free_r(jsonValue_t* value) {
	jsonArray_t array;
	jsonObject_t object;

	switch(value->type) {
		case JSON_NULL:
			break;
		case JSON_ARRAY:
			array = value->value.array;
			for (int i = 0; i < array.size; i++) {
				json_free_r(&(array.entries[i]));
			}
			free(array.entries);
			break;
		case JSON_OBJECT:
			object = value->value.object;
			for (int i = 0; i < object.size; i++) {
				free(object.entries[i].key);
				json_free_r(&(object.entries[i].value));
			}
			free(object.entries);
			break;
		case JSON_STRING:
			free(value->value.string);
			break;
		default:
			break;
	}
}

void json_free(jsonValue_t* value) {
	if (value == NULL)
		return;

	json_free_r(value);
	free(value);
}

jsonValue_t* json_value() {
	jsonValue_t* value = malloc(sizeof(jsonValue_t));
	return value;
}

jsonValue_t* json_null() {
	jsonValue_t* value = json_value();
	if (value == NULL)
		return NULL;
	value->type = JSON_NULL;
	return value;
}

jsonValue_t* json_double(double d) {
	jsonValue_t* value = json_value();
	if (value == NULL)
		return NULL;
	value->type = JSON_DOUBLE;
	value->value.real = d;
	return value;
}

jsonValue_t* json_long(long l) {
	jsonValue_t* value = json_value();
	if (value == NULL)
		return NULL;
	value->type = JSON_LONG;
	value->value.integer = l;
	return value;
}

jsonValue_t* json_bool(bool b) {
	jsonValue_t* value = json_value();
	if (value == NULL)
		return NULL;
	value->type = JSON_BOOL;
	value->value.boolean = b;
	return value;
}

jsonValue_t* json_string(const char* s) {
	jsonValue_t* value = json_value();
	if (value == NULL)
		return NULL;
	value->type = JSON_STRING;
	value->value.string = strdup(s);
	if (value->value.string == NULL) {
		free(value);
		return NULL;
	}
	return value;
}

jsonValue_t* json_array(bool freeAfterwards, size_t size, ...) {
	jsonValue_t* value = json_value();
	if (value == NULL)
		return NULL;
	value->type = JSON_ARRAY;
	value->value.array.size = size;
	value->value.array.entries = malloc(sizeof(jsonValue_t) * size);
	if (value->value.array.entries == NULL) {
		free(value);
		return NULL;
	}
	
	bool abort = false;
	
	va_list ap;
	va_start(ap, size);
	for (size_t i = 0; i < size; i++) {
		jsonValue_t* entry = va_arg(ap, jsonValue_t*);
		if (entry == NULL) {
			abort = true;
			if (freeAfterwards) {
				// we need to free each argument
				// this one is null -> don't free
				continue;
			} else {
				break;
			}
		}
		
		// if aborted this does nothing
		value->value.array.entries[i] = *entry;
		
		if (freeAfterwards) {
			free(entry);
		}
	}
	va_end(ap);
	
	if (abort) {
		free(value->value.array.entries);
		free(value);
		return NULL;
	}
	
	return value;
}

jsonValue_t* json_array_direct(bool freeAfterwards, size_t size, jsonValue_t* values[]) {
	jsonValue_t* value = json_value();
	if (value == NULL)
		return NULL;
	value->type = JSON_ARRAY;
	value->value.array.size = size;
	value->value.array.entries = malloc(sizeof(jsonValue_t) * size);
	if (value->value.array.entries == NULL) {
		free(value);
		return NULL;
	}
	
	bool abort = false;
	
	for (size_t i = 0; i < size; i++) {
		if ((values[i]) == NULL) {
			abort = true;
			if (freeAfterwards) {
				continue;
			} else {
				break;
			}
		}
		
		// if aborted this does nothing
		value->value.array.entries[i] = *(values[i]);
	
		if (freeAfterwards) {
			free(values[i]);
		}
	}
	
	if (abort) {
		free(value->value.array.entries);
		free(value);
		return NULL;
	}
	
	return value;
}

jsonValue_t* json_object(bool freeAfterwards, size_t size, ...) {
	jsonValue_t* value = json_value();
	if (value == NULL)
		return NULL;
	value->type = JSON_OBJECT;
	value->value.object.size = size;
	
	// using calloc so the memory is set to 0
	value->value.object.entries = calloc(size, sizeof(jsonObjectEntry_t));
	if (value->value.object.entries == NULL) {
		free(value);
		return NULL;
	}
	
	bool abort = false;
	
	va_list ap;
	va_start(ap, size);
	for (size_t i = 0; i < size; i++) {
		const char* key = va_arg(ap, const char*);
		jsonValue_t* entry = va_arg(ap, jsonValue_t*);
		
		if (key == NULL) {
			abort = true;
			if (!freeAfterwards) {
				break;
			}
		}
		if (entry == NULL) {
			abort = true;
			if (freeAfterwards) {
				continue;
			} else {
				break;
			}
		}
		
		if (!abort) {
			value->value.object.entries[i].key = strdup(key);
			value->value.object.entries[i].value = *entry;
		}
		if (freeAfterwards) {
			free(entry);
		}
	}
	va_end(ap);
	
	if (abort) {
		for (size_t i = 0; i < size; i++) {
			if (value->value.object.entries[i].key != NULL) {
				free(value->value.object.entries[i].key);
			}
		}
		free(value->value.object.entries);
		free(value);
		return NULL;
	}
	
	return value;
}

void print_repeat(int i, char c) {
	for (int j = 0; j < i; j++) {
		printf("%c", c);
	}
}

void json_print_r(jsonValue_t* value, int indent) {
	print_repeat(indent, '\t');
	if (value == NULL) {
		printf("[invalid]\n");
		return;
	}
	
	switch(value->type) {
		case JSON_ARRAY:
			printf("array:\n");
			for (size_t i = 0; i < value->value.array.size; i++) {
				json_print_r(&value->value.array.entries[i], indent + 1);
			}
			break;
		case JSON_OBJECT:
			printf("object:\n");
			for (size_t i = 0; i < value->value.object.size; i++) {
				print_repeat(indent + 1, '\t');
				printf("%s:\n", value->value.object.entries[i].key);
				json_print_r(&value->value.object.entries[i].value, indent + 2);
			}
			break;
		case JSON_DOUBLE:
			printf("number: %lf\n", value->value.real);
			break;
		case JSON_LONG:
			printf("number: %lld\n", value->value.integer);
			break;
		case JSON_STRING:
			printf("string: \"%s\"\n", value->value.string);
			break;
		case JSON_BOOL:
			printf("bool: %s\n", value->value.boolean ? "true" : "false");
			break;
		case JSON_NULL:
			printf("null\n");
			break;
	}
}

void json_print(jsonValue_t* value) {
	json_print_r(value, 0);
}

int json_clone_r(jsonValue_t* value, jsonValue_t* clone) {
	*clone = *value;
	
	switch(value->type) {
		case JSON_STRING:
			clone->value.string = strdup(value->value.string);
			if (clone->value.string == NULL) {
				return -1;
			}
			break;
		case JSON_ARRAY:
			clone->value.array.size = value->value.array.size;
			clone->value.array.entries = malloc(sizeof(jsonValue_t) * clone->value.array.size);
			
			if (clone->value.array.entries == NULL) {
				return -1;
			}
			
			for (size_t i = 0; i < clone->value.array.size; i++) {
				if (json_clone_r(&(value->value.array.entries[i]), &(clone->value.array.entries[i])) < 0) {
					free(clone->value.array.entries);
					return -1;
				}
			}
			
			break;
		case JSON_OBJECT:
			clone->value.object.size = value->value.object.size;
			clone->value.object.entries = malloc(sizeof(jsonObjectEntry_t) * clone->value.object.size);
			
			if (clone->value.object.entries == NULL) {
				return -1;
			}
			
			for (size_t i = 0; i < clone->value.object.size; i++) {
				bool okay = true;
			
				clone->value.object.entries[i].key = strdup(value->value.object.entries[i].key);
				if (clone->value.object.entries[i].key == NULL) {
					okay = false;
				} else if (json_clone_r(&(value->value.object.entries[i].value), &(clone->value.object.entries[i].value)) < 0) {
					free(clone->value.object.entries[i].key);
					okay = false;
				}
				
				if (!okay) {
					for (size_t j = 0; j < i; j++) {
						free(clone->value.object.entries[j].key);
						json_free_r(&(clone->value.object.entries[j].value));
					}
					free(clone->value.object.entries);
					return -1;
				}
			}
			
			break;
			
		default:
			// non dynamic members already copied
			break;
	}
	
	return 0;
}

jsonValue_t* json_clone(jsonValue_t* value) {
	jsonValue_t* clone = malloc(sizeof(jsonValue_t));
	if (clone == NULL) {
		return NULL;
	}
		
	if (json_clone_r(value, clone) < 0) {
		free(clone);
		return NULL;
	}
	
	return clone;
}
