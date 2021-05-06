# Cson

[![Test Suite](https://github.com/overflowerror/Cson/actions/workflows/test-suite.yml/badge.svg)](https://github.com/overflowerror/Cson/actions/workflows/test-suite.yml)

## JSON Library for C

The purpose of this project is to provide a convinient way to interact with JSON data in C.

Functions for constructing, interacting, encoding and decoding of JSON data can be accessed through the `json.h` header. Additionally this projects includes a marshaller generater that takes a struct definitions and generates functions to encode or decoded these structs.

### Disclaimer

This library was originaly just a finger exercise to see if I could build a JSON lib from scratch. That also means that I can't guarantee for anything - treat it as highly unstable.

Nevertheless you are welcome to use or modify the code as you like. I also appriciate pull requests.

## Dependencies

For the base functionallity there are no dependencies.

The marshaller generator needs `lex` and `yacc`. The `Makefile` uses `flex` and `bison` but I think I only used `lex` and `yacc` functionality respectively.
Also a `gcc` compatible compiler (or a compiler that supports GCC function attributes) is need. So `clang` should work too.

The `Makefile` is build for `gcc`

## Build

Static lib:
`make libcson.a`

Shared lib:
`make libcson.so`

Base Demo program:
`make json-demo`

Base Test program:
`make json-test`

Marshaller program:
`make marshaller-gen`

Marshaller Demo program:
`make marshaller-demo`

Marshaller Test program:
`make marshaller-test`

### Test

To run the test suit just run `make tests`

## Usage (Base Functionallity)

### General

All data types in the lib are supposed to be used as pointers. Every library function that retrieves data from a JSON value will return a new "object" (i.e. a clone) that is not coupled to the original one.
This means that every generated JSON value has to freed seperately. Not doing so will result in a memory leak.

To free any `jsonValue_t` the function `json_free(jsonValue_t*)` should be used. It will recursively release all resources currently held by the given value. The value can not be used afterwards.

### Interaction with Objects

The only data type that's relevant for a caller should be `jsonValue_t` which is a type that can hold all posible JSON values.

The only member that is important for interaction with the type is `.type`. This is an `enum` that can have one of the following values:

Type | Description
-----|------------
`JSON_LONG` | This represents an integer. The value of the integer can be accessed via the field `.value.integer`.
`JSON_DOUBLE` | This represents a floating point number. The value can be accessed via `.value.real`.
`JSON_STRING` | This is a string value. `.value.string` contains the `char*`. Note that all string in a `jsonValue_t` are memory managed by the lib and will be freed once the value is freed.
`JSON_BOOL` | A boolean value. It can be accessed with the field `.value.boolean`.
`JSON_NULL` | This is a null value. It doesn't have a corresponding C value.
`JSON_ARRAY` | This represents an array/list. To access it the library provides some functions (see Querying).
`JSON_OBJECT` | This is a JSON object. Similar to arrays the library provides functions to access it.

### Creation of Values

To create a `jsonValue_t` the following functions can be used.

- `jsonValue_t* json_long(long)`
- `jsonValue_t* json_double(double)`
- `jsonValue_t* json_string(const char*)`
- `jsonValue_t* json_bool(bool)`
- `jsonValue_t* json_array(bool, size_t, ...)`
- `jsonValue_t* json_object(bool, size_t, ...)`

#### Arrays

The second argument of `json_array()` is the number of entries in the array. All following arguments (va_args) are the value in the array. Note that the value arguments have to be `jsonValue_t*`.

Example:
```C
jsonValue_t* array = json_array(true, 4,
	json_long(1),
	json_long(2),
	json_long(3),
	json_long(4)
);
```

This will create a JSON array containing the numbers 1 through 4.

The first argument indicates to `json_array()` whether the value arguments should be freed after they are added to the array. Without this parameter being `true` the example above would cause a memory leak since `json_long()` will allocate memory on the heap.

#### Objects

Similar to `json_array()` the second argument of `json_object()` is the number of entries in the array. The following arguments are alternating keys (as strings) and values (as `jsonValue_t*`).

Example:
```C
jsonValue_t* object = json_object(true, 3,
	"foo", json_string("bar"),
	"bar", json_string("baz"),
	"baz", json_string("foo")
);
```

This will create a JSON object containing the keys "foo", "bar" and "baz" corresponding to the string values "bar", "baz", "foo".

As with `json_array()` the first arguments indicates if the values should be freed after adding them. Without this parameter being `true` the example above would cause a memory leak since `json_string()` will copy the argument string on to the heap.
The key arguments will be copied onto the heap as well. However they won't be freed regardless of the first parameter, so using string literals - like in the example - does not cause undefined behavior.

### Querying

To access JSON arrays and objects the following two functions are provided:

`jsonValue_t* json_array_get(jsonValue_t*, size_t)` will retrieve the nth value (second argument) from the the array (first argument). If the provided value is not an array, NULL is returned. If the index does not exist, a JSON null value (`json_null()`) is returned.

`jsonValue_t* json_object_get(jsonValue_t*, const char*)` will retrieve the corresponding value to the key (second argument) from the object (first argument). If the value is not an object, NULL is returned. If the key does not exist, a JSON null value (`json_null()`) is returned.

Note: For both of these functions the returned value will be a clone of the array entry or the object member. Meaning the result has to be freed seperately from the array/object itself.

#### Query Function

Additionally to those two functions there is also a query function that is much more powerful - but also much more expensive computationally.

`jsonValue_t* json_query(jsonValue_t*, const char*)` will return the matching value to the query string (second argument) in the array/object (first argument). If the provided value is neither an array nor an object, NULL is returned. If the query could not be parsed, NULL is returned. If the structure of the value doesn't match the query, NULL is returned. If the structure matches but a selected index/key is not available, a JSON null value (`json_null()`) is returned.

The syntax of the query string is loosly based on the `jq` syntax. The following grammar describes the query language.
```
query           := "." selector [query]
selector        := array_selector | object_selector
array_selector  := "[" index "]"
object_selector := key
```

Examples:

`.foobar` will select the corresponding value for the key "foobar" from an object

`.[4]` will select the 5th (index counting starts with 0) from an array

`.[0].foo` will select the key "foo" from the first entry in an array

`.foo.bar.[0]` will select the first element in the key "bar" in the key "foo" in an object


Note: As with the `json_array_get()` and `json_object_get()` the returned value is a clone and has to be freed seperately.

### Stringify

Using the `char* json_stringify(jsonValue_t*)` function a JSON value can be converted into a string.

The string will be stored on the heap and has to be freed manually.

### Miscellaneous

The function `json_print(jsonValue_t*)` will display the structure and types of the value in the terminal (stdout).

`json_free(jsonValue_t*)` is used to recursively free a JSON value.

### Demo

The file demo/json.c provides a few examples on how the library could be used.

To build the demo just use `make json-demo`

## Usage (Marshaller)

### Build

To build to marshaller generator use the following command:

`make marshaller-gen`

### Marshaller-Gen Synopsis

```
./marshaller-gen [-o OUTPUT_FILE] {INPUT_FILE}
```

The marshaller generator takes any number of `INPUT_FILE`S (C header files) that contain the struct definitions. If no input file is given `stdin` is used.

Using the `-o` option the `OUTPUT_FILE` for the generated C code can be specified. If no OUTPUT_FILE is given `stdout` is used.

### Input Specification

Although the `INPUT_FILE`s are just normal C header files the syntax is quite restricted. It is recommended to only put the struct definitions in the input files. Typedefs can be used as long as the struct definition is part of the typedef.

Examples:

```
// this is okay
struct name {};
```

```
// this is okay
typedef struct name {} name_t;
```

```
// this is okay
typedef struct {} name_t;
```

```
// this is not
struct name {};
typedef struct name name_t;

```
For this library a typedef is considered an alias of the struct type.


Comments and preprocessor statements will be ignored. Using other language constructs will cause a syntax error.

#### Supported Types

integer types: `char`, `short`, `int`, `long`, `long long`

float types: `float`, `double`

string types: `char*`, `const char*`

(While supported using `const` does not make sense since all data is stored on the heap.)

struct types: any struct or typedef struct type that has a marshaller

array types: any of the types above; see Lists for details

pointer types: only single pointers to types that are not lists are allowed (`struct s*` is okay, `int***` is not)

### Compiling The Output File

The marshaller generator assumes that `json.h` as well as all input files are in the include path. So make sure to compile the `OUTPUT_FILE` with the correct `-I` options.

The result has to be linked with `libcson.a` or `libcson.so`.

### Marshalling

To encode a struct as JSON just use the `json_marshall()` function (declared in `marshaller.h`). This function takes 2 arguments and returns the marshalled struct as a string.

The argument is the type of the struct. The second argument is a pointer to the struct.

Example:

Let's assume the struct looks like this:
```
struct s {
	int i;
	char* s;
};
```

Then the call to `json_marshall()` looks something like this:
```
// obj is the struct to be marshalled
struct s obj = { .i = 42, .s = "hello World" };

char* result = json_marshall(struct s, &obj);

free(result);
```

This also works if there is a typedef for the struct.

### Unmarshalling

To decode a JSON string into a struct the `json_unmarshall()` function is used. 

It works similar to `json_marshall()`. The difference is that here the second parameter is the input string and the result is a struct pointer.

Example (using the same struct definition as above):

```
struct s* obj = json_unmarshall(struct s, "{\"i\":1337,\"s\":\"foobar\"}");

json_free_struct(struct s, obj);
```

The function `json_free_struct()` is used to recursively free the resouses used in the struct.

### Lists

The marshaller can also deal with arrays (lists) - either as struct members or for marshalling/unmarshalling collections.

Note: To use lists as struct members they have to be declared as a double pointer of the type of the list member. For example: If it is an integer list the struct member has to be declared as `int**`.

Lists are represented as an array of pointers. Similar to `envoiron` the length of the list is given implicitly: The pointer after the last element is NULL.

Example:
```
const char* array[3];
array[0] = "foo";
array[1] = "bar";
array[2] = NULL;
```

To iterate over such a list use a for loop similar to this one:
```
for (size_t i = 0; array[i] != NULL; i++) {
	printf("%s\n", array[i]);
}
```

#### Marshalling

To encode a list use the `json_marshall_array()` function that works similar to `json_marshall()`. If a list is part of a struct the marshalling is handled by the struct marshaller.

#### Unmarshalling

To decode a list use the `json_unmarshall_array()` function that works similar to `json_unmarshall()`. If a list is part of a struct the unmarshalling is handled by the struct marshaller.

To free a decoded list use the `json_free_array()` function that works similar to `json_free_struct()`. If a list is part of a struct the freeing is handled by the struct marshaller.

If the list has a primitive type (i.e. not a struct, not a list; Note: For this strings are also considered primitive) use the `json_free_prim_array()` function. It only has the list as an argument.

### Demo

The file demo/marshaller.c provides an example on how the marshaller could be used.

To build the demo just use `make marshaller-demo`

### Error Messages

Message | Meaning
-----------------
`file limit reached` | By default marshaller-gen only accepts 10 input files. This can be changed in the constant `MAX_FILES` in codegen.c.
`lexical error in line` | The marshaller generator only supports a very limited syntax for the header file. Make sure only have structs or typedefs in the input files. For more details see Input Specification.
`syntax error` | See `lexical error in line`.
`... not yet supported` | The respective feature or type is not yet implemented.
`... not supported` | The respective feature or type is not supported and probably won't ever be.
`const char* struct members are discouraged` | The use of `const char*` doesn't make sense because all the data will be on the heap anyway. `const` would just confuse.
`double pointer type; assuming dynamic array` | Not an error. Just a hint that double pointer types will be assumed to be dynamic arrays - which may not be but the user intended.
`unknown type` | The marshaller for the given type is not present (not linked).
`marshaller for name already present` | The type has multiple marshallers.
