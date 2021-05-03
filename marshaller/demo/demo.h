#ifndef DEMO_H
#define DEMO_H


typedef struct {
	long long uid;
	const char* username;
	const char* email;
} user_t;

typedef struct {
	const char* name;
	const char* content;
	long* views;
	user_t user;
} post_t;

#endif
