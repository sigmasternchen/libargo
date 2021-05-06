#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "json.h"

extern void json_free_r(jsonValue_t* value);

struct parserToken {
	size_t length;
	char* token;
};

#define EMPTY_PARSER_TOKEN ((struct parserToken) { .length = 0, .token = NULL})

#define PARSER_TOKEN_CHUNK_SIZE (1024)

int addToParserToken(struct parserToken* token, char c) {
	if (token->length % PARSER_TOKEN_CHUNK_SIZE == 0) {
		char* tmp = realloc(token->token, sizeof(char) * (token->length / PARSER_TOKEN_CHUNK_SIZE + 1) * PARSER_TOKEN_CHUNK_SIZE);
		if (tmp == NULL) {
			free(token->token);
			token->token = NULL;
			return -1;
		}
		token->token = tmp;
	}
	
	token->token[token->length++] = c;
	
	return 0;
}

void freeParserToken(struct parserToken* token) {
	if (token->token != NULL) {
		free(token->token);
	}
}

typedef struct {
	bool okay;
	const char* errorFormat;
	size_t index;
	size_t line;
	jsonValue_t value;
} jsonParsedValue_t;

#define JSON_PARSER_STATE_IDLE       (0)

#define JSON_PARSER_STATE_STRING     (10)

#define JSON_PARSER_STATE_ARRAY      (20)

#define JSON_PARSER_STATE_OBJECT     (30)

#define JSON_PARSER_STATE_LONG       (40)

#define JSON_PARSER_STATE_DOUBLE     (50)

jsonParsedValue_t json_parse_long(jsonParsedValue_t value, struct parserToken token) {	
	if (addToParserToken(&token, '\0') < 0) {
		value.errorFormat = "internal error while parsing numgber";
		return value;
	}

	char* endptr;
	long long l = strtoll(token.token, &endptr, 10);
	if (*endptr != '\0') {
		value.index -= strlen(token.token) - (endptr - token.token);
		value.errorFormat = "line %ld: illegal character '%c'\n";
	} else {
		value.okay = true;
		value.value.type = JSON_LONG;
		value.value.value.integer = l;
	}

	return value;
}

jsonParsedValue_t json_parse_double(jsonParsedValue_t value, struct parserToken token) {	
	if (addToParserToken(&token, '\0') < 0) {	
		value.errorFormat = "internal error while parsing numgber";
		return value;
	}

	char* endptr;
	double d = strtod(token.token, &endptr);
	if (*endptr != '\0') {
		value.index -= strlen(token.token) - (endptr - token.token);
		value.errorFormat = "line %ld: illegal character '%c'";
	} else {
		value.okay = true;
		value.value.type = JSON_DOUBLE;
		value.value.value.real = d;
	}

	return value;
}

jsonParsedValue_t json_parse_r(const char* string, size_t index, size_t line, size_t length) {
	jsonParsedValue_t value;
	value.okay = false;
	value.line = line;
	
	int state = JSON_PARSER_STATE_IDLE;
	
	struct parserToken token = EMPTY_PARSER_TOKEN;
	
	bool escaped = false;
	bool readyForNext = true;
	char* key = NULL;
	
	for (; index < length; index++) {
		char c = string[index];
		
		if (c == '\n') {
			value.line++;
		}
		
		if (state != JSON_PARSER_STATE_STRING && (c == ' ' || c == '\t' || c == '\n')) {
			continue;
		}
		switch(state) {
			case JSON_PARSER_STATE_IDLE:
				switch(c) {
					case '"':
						state = JSON_PARSER_STATE_STRING;
						value.value.type = JSON_STRING;
						break;
					case '[':
						state = JSON_PARSER_STATE_ARRAY;
						value.value.type = JSON_ARRAY;
						value.value.value.array.size = 0;
						value.value.value.array.entries = NULL;
						break;
					case '{':
						state = JSON_PARSER_STATE_OBJECT;
						value.value.type = JSON_OBJECT;
						value.value.value.object.size = 0;
						value.value.value.object.entries = NULL;
						break;
					
					case '0':
					case '1':
					case '2':
					case '3':
					case '4':
					case '5':
					case '6':
					case '7':
					case '8':
					case '9':
					case '-':
						if (addToParserToken(&token, c) < 0) {
							value.errorFormat = "internal error in line %ld";
							return value;
						}
						state = JSON_PARSER_STATE_LONG;
						break;
					default:
						if (length - index >= strlen("null") && strncmp("null", string + index, strlen("null")) == 0) {
							value.okay = true;
							value.index = index + strlen("null");
							value.value.type = JSON_NULL;
						} else if (length - index >= strlen("true") && strncmp("true", string + index, strlen("true")) == 0) {
							value.okay = true;
							value.index = index + strlen("true");
							value.value.type = JSON_BOOL;
							value.value.value.boolean = true;
						} else if (length - index >= strlen("false") && strncmp("false", string + index, strlen("false")) == 0) {
							value.okay = true;
							value.index = index + strlen("false");
							value.value.type = JSON_BOOL;
							value.value.value.boolean = false;
						} else {
							value.index = index;
							value.errorFormat = "illegal character in line %d: '%c'";
						}
						
						freeParserToken(&token);
						
						return value;
				}
				break;
			case JSON_PARSER_STATE_STRING:
				if (!escaped && c == '"') {
					if (addToParserToken(&token, '\0') < 0) {
						value.errorFormat = "internal error while parsing string";
						return value;
					}
					value.index = index + 1;
					value.value.value.string = strdup(token.token);
					if (value.value.value.string == NULL) {
						value.errorFormat = "couldn't strdup while parsing string";
						
						freeParserToken(&token);
						return value;
					}
					
					value.okay = true;
					
					freeParserToken(&token);
					return value;
				}
				if (!escaped && c == '\\') {
					escaped = true;
					break;
				} else if (escaped) {
					int tmp = 0;
					switch(c) {
						case 'b':
							tmp = addToParserToken(&token, '\b');
							break;
						case 'f':
							tmp = addToParserToken(&token, '\f');
							break;
						case 'n':
							tmp = addToParserToken(&token, '\n');
							break;
						case 'r':
							tmp = addToParserToken(&token, '\r');
							break;
						case 't':
							tmp = addToParserToken(&token, '\t');
							break;
						case 'u':
							value.okay = false;
							value.index = index;
							value.errorFormat = "line %ld: \\u-syntax is not supported";
							freeParserToken(&token);
							return value;
							
						case '"':
						case '\\':
						case '/':					
							tmp = addToParserToken(&token, c);
							break;
							
						default:
							tmp = addToParserToken(&token, '\\');
							tmp = addToParserToken(&token, c);
							break;
					}
					if (tmp < 0) {
						value.errorFormat = "internal error while parsing string escape sequence";
						
						freeParserToken(&token);
						return value;
					}
					
					escaped = false;
				} else {
					switch(c) {
						case '\b':
						case '\f':
						case '\n':
						case '\r':
						case '\t':
							value.okay = false;
							value.index = index;
							value.errorFormat = "line %ld: control characters are not allowed in json strings";
				
							freeParserToken(&token);
							return value;
						default:
							if (addToParserToken(&token, c) < 0) {
								value.errorFormat = "internal error while parsing string";
								
								freeParserToken(&token);
								return value;
							}
							break;
					}
				}
				break;
			case JSON_PARSER_STATE_LONG:
				if (c >= '0' && c <= '9') {
					if (addToParserToken(&token, c) < 0) {
						value.errorFormat = "internal error while parsing number";
						
						freeParserToken(&token);
						return value;
					}
				} else if (c == '.' || c == 'e') {
					if (addToParserToken(&token, c) < 0) {
						value.errorFormat = "internal error while parsing number";
						
						freeParserToken(&token);
						return value;
					}
					state = JSON_PARSER_STATE_DOUBLE;
				} else {
					value.index = index;
					value = json_parse_long(value, token);
					
					freeParserToken(&token);
					return value;
				}
				break;
			case JSON_PARSER_STATE_DOUBLE:
				if ((c >= '0' && c <= '9') || c == '.' || c == '+' || c == '-' || c == 'e') {
					if (addToParserToken(&token, c) < 0) {
						value.errorFormat = "internal error while parsing number";
						
						freeParserToken(&token);
						return value;
					}
				} else {
					value.index = index;
					value = json_parse_double(value, token);
					
					freeParserToken(&token);
					return value;
				}
				break;
			case JSON_PARSER_STATE_ARRAY:
				{
					if (c == ']') {
						value.index = index + 1;
						value.okay = true;
						return value;	
					}
					if (c == ',') {
						if (readyForNext) {
							json_free_r(&value.value);
						
							value.errorFormat = "line %ld: unexpected '%c'";
							value.index = index;
							
							freeParserToken(&token);
							return value;
						}
						
						readyForNext = true;
						
						break;
					}
					
					if (!readyForNext) {	
						json_free_r(&value.value);
					
						value.errorFormat = "line %ld: unexpected '%c'; ',' or ']' expected";
						value.index = index;
						return value;
					}
					
					jsonParsedValue_t entry = json_parse_r(string, index, value.line, length);
					
					if (!entry.okay) {
						json_free_r(&value.value);
						
						freeParserToken(&token);
						return entry;
					}
					
					jsonValue_t* entries = realloc(value.value.value.array.entries, sizeof(jsonValue_t) * (value.value.value.array.size + 1));
					
					if (entries == NULL) {
						json_free_r(&value.value);
						
						value.errorFormat = "allocation for array failed";
						
						freeParserToken(&token);
						return value;
					}
					
					entries[value.value.value.array.size++] = entry.value;
					
					value.value.value.array.entries = entries;
					
					index = entry.index - 1;
					
					readyForNext = false;
				}
				break;
			case JSON_PARSER_STATE_OBJECT:
				{
					if (c == '}') {
						value.index = index + 1;
						value.okay = true;
						return value;	
					}
					if (c == ',') {
						if (readyForNext) {
							json_free_r(&value.value);
						
							value.errorFormat = "line %ld: unexpected ','";
							value.index = index;
							return value;
						}
						
						if (key != NULL) {
							json_free_r(&value.value);
						
							value.errorFormat = "line %ld: unexpected ','; ':' expected";
							value.index = index;
							
							freeParserToken(&token);
							return value;
						}
						
						readyForNext = true;
						
						break;
					}
					
					if (c == ':') {
						if (readyForNext) {
							json_free_r(&value.value);
						
							value.errorFormat = "line %ld: unexpected character ':'";
							value.index = index;
							
							freeParserToken(&token);
							return value;
						}
						
						if (key == NULL) {
							json_free_r(&value.value);
						
							value.errorFormat = "line %ld: unexpected ':'; key is missing";
							value.index = index;
							
							freeParserToken(&token);
							return value;
						}
						
						readyForNext = true;
						
						break;
					}
					
					if (!readyForNext) {	
						json_free_r(&value.value);
					
						value.errorFormat = "line %ld: unexpected '%c'; ',' or '}' expected";
						value.index = index;
						
						freeParserToken(&token);
						return value;
					}
					
					jsonParsedValue_t entry = json_parse_r(string, index, value.line, length);
					
					if (!entry.okay) {
						json_free_r(&value.value);
						
						freeParserToken(&token);
						return entry;
					}
					
					if (key == NULL) {
						if (entry.value.type != JSON_STRING) {
							json_free_r(&value.value);
							json_free_r(&entry.value);
							
							value.errorFormat = "line %ld: key is missing";
							value.index = index;
							
							freeParserToken(&token);
							return value;
						}
						
						key = entry.value.value.string;
					} else {
					
						jsonObjectEntry_t* entries = realloc(value.value.value.array.entries, sizeof(jsonObjectEntry_t) * (value.value.value.object.size + 1));
						
						if (entries == NULL) {
							json_free_r(&value.value);
							
							value.errorFormat = "allocation for object failed";
							
							freeParserToken(&token);
							return value;
						}
						
						entries[value.value.value.object.size].key = key;
						entries[value.value.value.object.size++].value = entry.value;
						
						value.value.value.object.entries = entries;
						
						key = NULL;
						
					}
					
					index = entry.index - 1;
					
					readyForNext = false;
				}
				break;
			default:
				value.index = index;
				value.errorFormat = "illegal state in line %ld";
				
				freeParserToken(&token);
				return value;
		}
	}
	
	if (state == JSON_PARSER_STATE_LONG) {
		value.index = index;
		value = json_parse_long(value, token);
		
		freeParserToken(&token);
		return value;
	}
	if (state == JSON_PARSER_STATE_DOUBLE) {
		value.index = index;
		value = json_parse_double(value, token);
		
		freeParserToken(&token);
		return value;
	}
	
	value.index = index - 1;
	value.errorFormat = "unexpected end of input on line %ld";
	
	freeParserToken(&token);
	return value;
}

jsonValue_t* json_parse(const char* string) {
	size_t length = strlen(string);

	jsonParsedValue_t parsedValue = json_parse_r(string, 0, 1, length);
	
	if (!parsedValue.okay) {
		// TODO put in extern global instead
		printf("%ld\n", parsedValue.index);
		printf(parsedValue.errorFormat, parsedValue.line, string[parsedValue.index]);
		printf("\n");
		return NULL;
	}
	
	if (length != parsedValue.index) {
		printf("unexptected character '%c' in line %ld\n", string[parsedValue.index], parsedValue.line);
		return NULL;
	}
	
	jsonValue_t* value = malloc(sizeof(jsonValue_t));
	if (value == NULL) {
		json_free_r(&(parsedValue.value));
		return NULL;
	}
	
	*value = parsedValue.value;
	
	return value;
}
