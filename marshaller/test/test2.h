#ifndef TEST2_H
#define TEST2_H

typedef struct {
	int intValue;
} struct_t;

typedef struct {
	int intValue;
	long longValue;
	
	double doubleValue;
	float floatValue;
	
	bool boolValue;
	
	const char* stringValue;
	char* stringValue2;
	
	struct_t structValue;
	
	int* intPointer;
	struct_t* structPointer;
} all_t;


#endif
