#include <stdio.h>
#include <alloca.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>

#include "json.h"


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
	checkBool(value == compare, check);
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

void testDouble() {
	double d;
	d = 3.1415926;
	
	jsonValue_t* v = json_double(d);
	
	checkNull(v, "result is not null");
	checkInt(v->type, JSON_DOUBLE, "type is correct");
	checkDouble(v->value.real, d, "value is correct");
	
	char* string = json_stringify(v);
	
	size_t size = snprintf(NULL, 0, "%lf", d);
	char* compare = alloca(size + 1);
	sprintf(compare, "%lf", d);
	
	checkString(string, compare, "stringify");
	
	free(string);
	json_free(v);
}

void testLong() {
	long long l;
	l = 1337;
	
	jsonValue_t* v = json_long(l);
	
	checkNull(v, "result is not null");
	checkInt(v->type, JSON_LONG, "type is correct");
	checkInt(v->value.integer, l, "value is correct");
	
	char* string = json_stringify(v);
	
	size_t size = snprintf(NULL, 0, "%lld", l);
	char* compare = alloca(size + 1);
	sprintf(compare, "%lld", l);
	
	checkString(string, compare, "stringify");
	
	free(string);
	json_free(v);
}

void testBool() {
	bool b;
	b = true;
	
	jsonValue_t* v = json_bool(b);
	
	checkNull(v, "result is not null");
	checkInt(v->type, JSON_BOOL, "type is correct");
	checkBool(v->value.boolean, "value is correct");
	
	char* string = json_stringify(v);
	char* compare = "true";
	
	checkString(string, compare, "stringify");
	
	free(string);
	json_free(v);
	
	b = false;
	
	v = json_bool(b);
	
	checkNull(v, "result is not null");
	checkInt(v->type, JSON_BOOL, "type is correct");
	checkBool(!v->value.boolean, "value is correct");
	
	string = json_stringify(v);
	compare = "false";
	
	checkString(string, compare, "stringify");
	
	free(string);
	json_free(v);
}

void testString() {
	const char* s = "foobar";
	
	jsonValue_t* v = json_string(s);
	
	checkNull(v, "result is not null");
	checkInt(v->type, JSON_STRING, "type is correct");
	
	char* string = json_stringify(v);
	
	char* compare = alloca(strlen(s) + 2 + 1);
	sprintf(compare, "\"%s\"", s);
	
	checkString(string, compare, "stringify");
	
	free(string);
	json_free(v);
}

void testNull() {
	jsonValue_t* v = json_null();
	
	checkNull(v, "result is not null");
	checkInt(v->type, JSON_NULL, "type is correct");
	
	char* string = json_stringify(v);
	
	char* compare = "null";
	
	checkString(string, compare, "stringify");
	
	free(string);
	json_free(v);
}

void testArray() {
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
	
	checkNull(value, "result is not null");
	checkInt(value->type, JSON_ARRAY, "type is correct");
	
	checkInt(value->value.array.size, 4, "array length is correct");
	checkInt(value->value.array.entries[0].type, JSON_STRING, "[0] type is correct");
	checkInt(value->value.array.entries[1].type, JSON_STRING, "[1] type is correct");
	checkInt(value->value.array.entries[2].type, JSON_NULL, "[2] type is correct");
	checkInt(value->value.array.entries[3].type, JSON_OBJECT, "[3] type is correct");
	
	char* string = json_stringify(value);
	// the two zeros are technically not wrong but kinda hard to remove => let's keep them
	char* compare = "[\"Hello\",\"World\",null,{\"okay\":true,\"pi\":3.141500,\"leet\":1337}]";
	
	checkString(string, compare, "stringify");
	
	free(string);
	json_free(value);
}

void testObject() {
	jsonValue_t* value = json_object(true, 3,
		"foo", json_string("bar"),
		"number", json_long(42),
		"list", json_array(true, 3,
			json_bool(true),
			json_double(3.1415),
			json_null()
		)
	);
	
	checkNull(value, "result is not null");
	checkInt(value->type, JSON_OBJECT, "type is correct");
	
	checkInt(value->value.object.size, 3, "object length is correct");
	
	checkString(value->value.object.entries[0].key, "foo", "[0] key is correct");
	checkString(value->value.object.entries[1].key, "number", "[1] key is correct");
	checkString(value->value.object.entries[2].key, "list", "[2] key is correct");
	
	checkInt(value->value.object.entries[0].value.type, JSON_STRING, "[0] type is correct");
	checkInt(value->value.object.entries[1].value.type, JSON_LONG, "[1] type is correct");
	checkInt(value->value.object.entries[2].value.type, JSON_ARRAY, "[2] type is correct");
	
	char* string = json_stringify(value);
	char* compare = "{\"foo\":\"bar\",\"number\":42,\"list\":[true,3.141500,null]}";
	
	checkString(string, compare, "stringify");
	
	free(string);
	json_free(value);
}

void testParse() {
	jsonValue_t* value = json_parse("{ \"foo\": \"bar\", \"foobar\": [ 1337, 3.1415, null, false] }");
	
	checkNull(value, "result is not null");
	checkInt(value->type, JSON_OBJECT, "type is correct");
	checkInt(value->value.object.size, 2, "object length is correct");
	
	checkString(value->value.object.entries[0].key, "foo", "[0] key is correct");
	checkString(value->value.object.entries[1].key, "foobar", "[1] key is correct");
	
	checkInt(value->value.object.entries[0].value.type, JSON_STRING, "[0] type is correct");
	checkInt(value->value.object.entries[1].value.type, JSON_ARRAY, "[1] type is correct");
	
	checkString(value->value.object.entries[0].value.value.string, "bar", "[0] value is correct");
	
	checkInt(value->value.object.entries[1].value.value.array.size, 4, "[0] array length is correct");
	
	checkInt(value->value.object.entries[1].value.value.array.entries[0].type, JSON_LONG, "[0] type is correct");
	checkInt(value->value.object.entries[1].value.value.array.entries[1].type, JSON_DOUBLE, "[1] type is correct");
	checkInt(value->value.object.entries[1].value.value.array.entries[2].type, JSON_NULL, "[2] type is correct");
	checkInt(value->value.object.entries[1].value.value.array.entries[3].type, JSON_BOOL, "[3] type is correct");
	
	checkInt(value->value.object.entries[1].value.value.array.entries[0].value.integer, 1337, "[0][0] value is correct");
	checkDouble(value->value.object.entries[1].value.value.array.entries[1].value.real, 3.1415, "[0][1] value is correct");
	checkBool(!value->value.object.entries[1].value.value.array.entries[3].value.boolean, "[0][3] value is correct");
	
	json_free(value);
}

void testQuery() {
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
	
	jsonValue_t* tmp;
	
	tmp = json_query(value, ".[0]");
	checkNull(tmp, "in array, not null");
	checkInt(tmp->type, JSON_STRING, "in array, type");
	checkString(tmp->value.string, "Hello", "in array, value");
	json_free(tmp);
	
	tmp = json_query(value, ".[3].okay");
	checkNull(tmp, "in obj in array, not null");
	checkInt(tmp->type, JSON_BOOL, "in obj in array, type");
	checkBool(tmp->value.boolean, "in obj in array, value");
	json_free(tmp);
	
	tmp = json_query(value, ".[4]");
	checkNull(tmp, "not in array, not null");
	checkInt(tmp->type, JSON_NULL, "not in array, type");
	
	json_free(tmp);
	
	tmp = json_query(value, ".[3].foobar");
	checkNull(tmp, "not in obj in array, not null");
	checkInt(tmp->type, JSON_NULL, "not in obj in array, type");
	
	json_free(tmp);
	
	json_free(value);
}

void testClone() {
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
	
	jsonValue_t* cloned = json_clone(value);
	
	checkNull(cloned, "clone not null");
	checkBool(((long long) value) != ((long long) cloned), "different addr");
	
	jsonValue_t* valueMember = &(value->value.array.entries[3]);
	jsonValue_t* cloneMember = &(cloned->value.array.entries[3]);
	
	checkNull(cloneMember, "member clone not null");
	checkBool(((long long) valueMember) != ((long long) cloneMember), "member different addr");
	
	json_free(cloned);
	json_free(value);
}

int main(int argc, char** argv) {
	header("Types");
	test("double", &testDouble);
	test("long", &testLong);
	test("bool", &testBool);
	test("string", &testString);
	test("null", &testNull);
	test("array", &testArray);
	test("array", &testObject);
	
	header("Functionality");
	test("parse", &testParse);
	test("query", &testQuery);
	test("clone", &testClone);
	


	printf("\nOverall: %s\n", overall ? "OK" : "FAILED");
	
	return overall ? 0 : 1;
}
