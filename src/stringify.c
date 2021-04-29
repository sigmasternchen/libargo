#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "json.h"

size_t string_escaped_length(const char* string) {
	size_t length = 0;
	char c;
	size_t i;
	for (i = 0, c = string[i]; c != '\0'; c = string[++i]) {
		switch(c) {
			case '\\':
			case '"':
			case '/':
			case '\b':
			case '\f':
			case '\n':
			case '\r':
			case '\t':
				length += 2;
				break;
			default:
				length += 1;
				break;
		}
	}
	
	return length;
}

size_t json_length(jsonValue_t* value) {
	size_t result = 0;
	switch (value->type) {
		case JSON_NULL:
			return 4;
		case JSON_STRING:
			return 2 + string_escaped_length(value->value.string);
		case JSON_DOUBLE:
			return snprintf(NULL, 0, "%lf", value->value.real);
		case JSON_LONG:
			return snprintf(NULL, 0, "%lld", value->value.integer);
		case JSON_BOOL:
			return 5;
		case JSON_ARRAY:
			result += 2;
			for (size_t i = 0; i < value->value.array.size; i++) {
				result += json_length(&(value->value.array.entries[i])) + 1;
			}
			return result;
		case JSON_OBJECT:
			result += 2;
			for (size_t i = 0; i < value->value.object.size; i++) {
				result += strlen(value->value.object.entries[i].key) + 2 + 1;
				result += json_length(&(value->value.object.entries[i].value)) + 1;
			}
			return result;
		default:
			return 0;
	}
}

size_t json_write_string(char* target, size_t maxSize, const char* source) {	
	
	#define JSON_WRITE_STRING_ADD(c) target[size++] = c; if (size == maxSize) return size;

	size_t size = 0;

	JSON_WRITE_STRING_ADD('"');
	
	size_t i;
	char c;
	for (i = 0, c = source[i]; c != '\0'; c = source[++i]) {
		switch(c) {
			case '\\':
				JSON_WRITE_STRING_ADD('\\');
				JSON_WRITE_STRING_ADD('\\');
				break;
			case '"':
				JSON_WRITE_STRING_ADD('\\');
				JSON_WRITE_STRING_ADD('"');
				break;
			case '/':
				JSON_WRITE_STRING_ADD('\\');
				JSON_WRITE_STRING_ADD('/');
				break;
			case '\b':
				JSON_WRITE_STRING_ADD('\\');
				JSON_WRITE_STRING_ADD('b');
				break;
			case '\f':
				JSON_WRITE_STRING_ADD('\\');
				JSON_WRITE_STRING_ADD('f');
				break;
			case '\n':
				JSON_WRITE_STRING_ADD('\\');
				JSON_WRITE_STRING_ADD('n');
				break;
			case '\r':
				JSON_WRITE_STRING_ADD('\\');
				JSON_WRITE_STRING_ADD('r');
				break;
			case '\t':
				JSON_WRITE_STRING_ADD('\\');
				JSON_WRITE_STRING_ADD('t');
				break;
			default:
				JSON_WRITE_STRING_ADD(c);
				break;
		}
	}
	
	JSON_WRITE_STRING_ADD('"');
	
	return size;
}

size_t json_stringify_r(char* string, size_t index, size_t totalSize, jsonValue_t* value) {
	switch(value->type) {
		case JSON_NULL:
			return snprintf(string + index, totalSize - index, "null") + index;
		case JSON_STRING:
			return json_write_string(string + index, totalSize - index, value->value.string) + index;
		case JSON_DOUBLE:
			return snprintf(string + index, totalSize - index, "%lf", value->value.real) + index;
		case JSON_LONG:
			return snprintf(string + index, totalSize - index, "%lld", value->value.integer) + index;
		case JSON_BOOL:
			return snprintf(string + index, totalSize - index, "%s", value->value.boolean ? "true" : "false") + index;
		case JSON_ARRAY:
			index += snprintf(string + index, totalSize - index, "[");
			
			for (size_t i = 0; i < value->value.array.size; i++) {
				index = json_stringify_r(string, index, totalSize, &(value->value.array.entries[i]));
			
				index += snprintf(string + index, totalSize - index, ",");
			}
			index--;
			
			index += snprintf(string + index, totalSize - index, "]");
			return index;
		case JSON_OBJECT:
			index += snprintf(string + index, totalSize - index, "{");
			
			for (size_t i = 0; i < value->value.object.size; i++) {
				index += json_write_string(string + index, totalSize - index, value->value.object.entries[i].key);
				index += snprintf(string + index, totalSize - index, ":");
				index = json_stringify_r(string, index, totalSize, &(value->value.object.entries[i].value));
			
				index += snprintf(string + index, totalSize - index, ",");
			}
			index--; // replace last , with }
			index += snprintf(string + index, totalSize - index, "}");
			return index;
		default:
			return index;
	}
}

char* json_stringify(jsonValue_t* value) {
	size_t size = json_length(value) + 1;

	char* string = malloc(size);
	if (string == NULL)
		return NULL;
		
	json_stringify_r(string, 0, size, value);		

	return string;
}
