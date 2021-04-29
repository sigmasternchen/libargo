#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>

#include "json.h"

int main() {
	jsonValue_t* value = json_array(true, 4,
		json_string("Hello"),
		json_string("World"),
		json_null(),
		json_object(true, 3,
			"okay", json_bool(true),
			"pi", json_double(3.1415),
			"leet", json_long(1337)
		)
	);
	
	json_print(value);
	
	printf("\n");
	char* string = json_stringify(value);
	printf("%s\n", string);
	
	free(string);
	
	printf("\n\n");
	
	jsonValue_t* tmp = json_query(value, ".[3].okay");
	
	json_print(tmp);
	
	json_free(tmp);
	json_free(value);
	
	printf("\n\n");
	
	value = json_parse("{ \"foo\": \"bar\", \"foobar\": [ 1337, 3.1415, null, false] }");
	
	json_print(value);
	
	json_free(value);

	return 0;
}
