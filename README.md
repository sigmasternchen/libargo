# Cson
## JSON Library for C

This library was originaly just a finger exercise to see if I could build a JSON lib from scratch. That also means that I can't guarantee for anything - treat it as highly unstable.

Nevertheless you are welcome to use or modify the code as you like. I also appriciate pull requests.

## Dependencies

None.

The `Makefile` is build for `gcc` but did not use any GNU-specific functionality so `clang` - or any other C-compiler for that matter - would probably work as well.

## Build

Static lib:
`make libcson.a`

Shared lib:
`make libcson.so`

Demo program:
`make demo`

Test program:
`make test`

### Test

To run the test suit just build the test program (see above) and run `./test`

## Usage

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

## Demo

The file src/demo.c provides a few examples on how the library could be used.

