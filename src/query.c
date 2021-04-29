#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "json.h"

jsonValue_t* json_object_get(jsonValue_t* value, const char* key) {
	if (value->type != JSON_OBJECT)
		return NULL;

	for (size_t i = 0; i < value->value.object.size; i++) {
		if (strcmp(value->value.object.entries[i].key, key) == 0) {
			return json_clone(&value->value.object.entries[i].value);
		}
	}
	
	return json_null();
}

jsonValue_t* json_array_get(jsonValue_t* value, size_t i) {
	if (value->type != JSON_ARRAY)
		return NULL;
		
	if (value->value.array.size <= i) {
			return json_null();
	}
	
	return json_clone(&value->value.array.entries[i]);
}

jsonValue_t* json_query(jsonValue_t* value, const char* query) {
	#define JSON_QUERY_BUFFER_SIZE (1024)

	char buffer[JSON_QUERY_BUFFER_SIZE];

	value = json_clone(value);

	while(true) {	
		if (query[0] == '\0')
			break;
			
		if (query[0] != '.') {
			json_free(value);
			return NULL;
		}
	
		size_t length;
		for (length = 1; query[length] != '\0' && query[length] != '.'; length++);
		
		if (length >= JSON_QUERY_BUFFER_SIZE) {
			json_free(value);
			return NULL;
		}
		
		memcpy(buffer, query, length);
		buffer[length] = '\0';
		
		query = query + length;
		
		if (strcmp(buffer, ".") == 0) {
			continue;
		}
		
		char* _buffer = buffer + 1;
		length--;
		
		switch(value->type) {
			case JSON_ARRAY:
				if (_buffer[0] != '[' || _buffer[length - 1] != ']') {
					json_free(value);
					return NULL;
				}
				_buffer[length - 1] = '\0';
				_buffer = _buffer + 1;
				
				char* endptr;
				long long index = strtoll(_buffer, &endptr, 10);
				if (*endptr != '\0') {
					json_free(value);
					return NULL;
				}
				if (index < 0) {
					json_free(value);
					return NULL;
				}
				
				jsonValue_t* arrayEntry = json_array_get(value, index);
				json_free(value);
				value = arrayEntry;
			
				break;
			case JSON_OBJECT:
				if (_buffer[0] == '"') {
					if (_buffer[length - 1] != '"') {
						json_free(value);
						return NULL;
					}
										
					_buffer[length - 1] = '\0';
					_buffer = _buffer + 1;
				}
				
				jsonValue_t* objectEntry = json_object_get(value, _buffer);
				json_free(value);
				value = objectEntry;
			
				break;
			default:
				json_free(value);
				return NULL;
		}
		
		if (value == NULL)
			return NULL;
	}
	
	return value;
}
