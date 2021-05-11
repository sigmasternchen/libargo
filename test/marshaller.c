#include <stdio.h>
#include <alloca.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>

#include <json.h>

#include <marshaller.h>

#include "test1.h"
#include "test2.h"


bool global = true;
bool overall = true;

void checkBool(bool ok, const char* check) {
	const char* result;
	if (ok) {
		result = "[  OK  ]";
	} else {
		result = "[FAILED]";
		global = false;
	}

	printf("%s:%*s%s\n", check, (int) (30 - strlen(check)), "", result);
}
void checkInt(long long value, long long compare, const char* check) {
	checkBool(value == compare, check);
}
void checkDouble(double value, double compare, const char* check) {
	checkBool(abs(value - compare) < 0.0000001, check);
}
void checkString(const char* value, const char* compare, const char* check) {
	checkBool(strcmp(value, compare) == 0, check);
}
void checkVoid(const void* value, const void* compare, const char* check) {
	checkBool(value == compare, check);
}
void checkNull(void* value, const char* check) {
	checkBool(value != NULL, check);
}

void showError() {
	fprintf(stderr, "Error: %s\n", strerror(errno));
}

/*bool hasData(int fd) {
	int tmp = poll(&(struct pollfd){ .fd = fd, .events = POLLIN }, 1, 10);

	return tmp == 1;
}*/

void test(const char* name, void (*testFunction)()) {
	printf("%s\n", name);
	printf("%.*s\n", (int) strlen(name), 
		"===================================");
	testFunction();
	if (!global)
		overall = false;
	printf("%s: %s\n\n", name, global ? "OK" : "FAILED");
	global = true;
}

void header(const char* text) {
	printf("\n");
	printf("=======================================\n");
	printf("== %s\n", text);
	printf("=======================================\n");
}

void testParsing() {
	char* json = "{\
		\"intValue\": 42,\
		\"longValue\": 1337,\
		\"doubleValue\": 3.14159265,\
		\"floatValue\": 3.16227766,\
		\"boolValue\": true,\
		\"stringValue\": \"foo\",\
		\"stringValue2\": \"bar\",\
		\"structValue\": {\
			\"intValue\": 2021\
		},\
		\"intPointer\": null,\
		\"structPointer\": null\
	}";
	
	all_t* all = json_unmarshall(all_t, json);
	
	checkNull(all, "result not null");
	if (all == NULL) {
		showError();
		return;
	}
	checkInt(all->intValue, 42, "int value");
	checkInt(all->longValue, 1337, "long value");
	checkDouble(all->doubleValue, 3.14159265, "double value");
	checkDouble(all->floatValue, 3.16227766, "float value");
	checkBool(all->boolValue, "bool value");
	checkString(all->stringValue, "foo", "string value");
	checkString(all->stringValue2, "bar", "string value 2");
	checkInt(all->structValue.intValue, 2021, "sub struct value");
	checkBool(all->intPointer == NULL, "int pointer null");
	checkBool(all->structPointer == NULL, "struct pointer null");
	
	json_free_struct(all_t, all);
	
	json = "{\
		\"intValue\": 0,\
		\"longValue\": 0,\
		\"doubleValue\": 0,\
		\"floatValue\": 0,\
		\"boolValue\": false,\
		\"stringValue\": \"\",\
		\"stringValue2\": \"\",\
		\"structValue\": {\
			\"intValue\": 0\
		},\
		\"intPointer\": 1337,\
		\"structPointer\": {\
			\"intValue\": 42\
		}\
	}";
	
	all = json_unmarshall(all_t, json);
	
	checkNull(all, "result not null");
	if (all == NULL) {
		showError();
		return;
	}
	
	checkBool(!all->boolValue, "bool value");
	checkNull(all->intPointer, "int pointer not null");
	checkNull(all->structPointer, "struct pointer not null");
	checkInt(*all->intPointer, 1337, "int pointer value");
	checkInt(all->structPointer->intValue, 42, "struct pointer value");
	
	json_free_struct(all_t, all);
}

void testStringify() {
	all_t all = {
		.intValue = 42,
		.longValue = 1337,
		.doubleValue = 3.141592,
		.floatValue = 3.162277,
		.boolValue = true,
		.stringValue = "foo",
		.stringValue2 = "bar",
		.structValue = {
			.intValue = 2021
		},
		.intPointer = NULL,
		.structPointer = NULL,
	};
	
	const char* compare = "{\"intValue\":42,\"longValue\":1337,\"doubleValue\":3.141592,\"floatValue\":3.162277,\"boolValue\":true,\"stringValue\":\"foo\",\"stringValue2\":\"bar\",\"structValue\":{\"intValue\":2021},\"intPointer\":null,\"structPointer\":null}";
	
	char* json = json_marshall(all_t, &all);
	
	//printf("%s\n", json);
	//printf("%s\n", compare);
	checkString(json, compare, "marshall (no pointers)");
	
	free(json);
	
	int tmpInt = 64;
	struct_t tmpStruct = {
		.intValue = 128
	};
	
	all.intPointer = &tmpInt;
	all.structPointer = &tmpStruct;
	
	compare = "{\"intValue\":42,\"longValue\":1337,\"doubleValue\":3.141592,\"floatValue\":3.162277,\"boolValue\":true,\"stringValue\":\"foo\",\"stringValue2\":\"bar\",\"structValue\":{\"intValue\":2021},\"intPointer\":64,\"structPointer\":{\"intValue\":128}}";
	
	json = json_marshall(all_t, &all);
	
	checkString(json, compare, "marshall (pointers)");
	
	free(json);
}

void testRecursive() {
	char* json = "{\
		\"i\": 1,\
		\"r\": {\
			\"i\": 2,\
			\"r\": {\
				\"i\": 3,\
				\"r\": null\
			}\
		}\
	}";
	
	struct recursive* _r = json_unmarshall(struct recursive, json);
	struct recursive* r = _r;
	
	int level = 1;
	while (r != NULL) {
		checkInt(r->i, level++, "level ok");
		r = r->r;
	}
	
	checkInt(level, 4, "total level count okay");
	
	r = _r;
	r->r->r->r = malloc(sizeof(struct recursive));
	
	r->r->r->r->i = level;
	r->r->r->r->r = NULL;
	
	char* result = json_marshall(struct recursive, r);
	const char* compare = "{\"i\":1,\"r\":{\"i\":2,\"r\":{\"i\":3,\"r\":{\"i\":4,\"r\":null}}}}";
	
	//printf("%s\n", result);
	//printf("%s\n", compare);
	
	checkString(result, compare, "result ok");
	
	json_free_struct(struct recursive, r);
	free(result);
}

void testArrays() {
	const char* json = "[1,2,3,4]";
	
	int** prim = json_unmarshall_array(int, json);
	
	checkNull(prim, "decode prim, not null");
	checkNull(prim[0], "decode prim[0], not null");
	checkInt(*prim[0], 1, "decode prim[0], value");
	checkNull(prim[1], "decode prim[1], not null");
	checkInt(*prim[1], 2, "decode prim[1], value");
	checkNull(prim[2], "decode prim[2], not null");
	checkInt(*prim[2], 3, "decode prim[2], value");
	checkNull(prim[3], "decode prim[3], not null");
	checkInt(*prim[3], 4, "decode prim[3], value");
	checkBool(prim[4] == NULL, "decode prim[4], null");
	
	json_free_prim_array(prim);
	
	int length = 6;
	prim = alloca(sizeof(int*) * (length + 1));
	int* primValues = alloca(sizeof(int) * length);
	for (int i = 0; i < length; i++) {
		primValues[i] = i;
		prim[i] = &primValues[i];
	}
	prim[length] = NULL;
	
	char* result = json_marshall_array(int, prim);
	
	checkNull(result, "encode prim, not null");
	checkString(result, "[0,1,2,3,4,5]", "encode prim, value");
	
	free(result);
	
	json = "[{\"intValue\":42},{\"intValue\":1337}]";
	
	struct_t** array = json_unmarshall_array(struct_t, json);
	
	checkNull(array, "decode struct, not null");
	checkNull(array[0], "decode struct[0], not null");
	checkInt(array[0]->intValue, 42, "decode struct[0], value");
	checkNull(array[1], "decode struct[1], not null");
	checkInt(array[1]->intValue, 1337, "decode struct[1], value");
	checkBool(array[2] == NULL, "decode struct[2], null");
	
	json_free_array(struct_t, array);
	
	array = alloca(sizeof(struct_t) * 1);
	struct_t value = { .intValue = 2021 };
	array[0] = &value;
	array[1] = NULL;
	
	result = json_marshall_array(struct_t, array);
	
	checkNull(result, "encode struct, not null");
	checkString(result, "[{\"intValue\":2021}]", "encode struct, value");
	
	free(result);
}

int main(int argc, char** argv) {
	test("parsing", &testParsing);
	test("stringify", &testStringify);
	test("recursive", &testRecursive);
	test("arrays", &testArrays);

	printf("\nOverall: %s\n", overall ? "OK" : "FAILED");
	
	return overall ? 0 : 1;
}
