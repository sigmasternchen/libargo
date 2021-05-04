#ifndef DEMO_H
#define DEMO_H


typedef struct {
	long long uid;
	char* username;
	char* email;
} user_t;

typedef struct {
	char* name;
	char* content;
	long* views;
	user_t user;
} post_t;

#endif
