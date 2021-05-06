#include <stdio.h>
#include <stdlib.h>
#include <alloca.h>

#include <json.h>
#include <marshaller.h>

#include "demo.h"


int main() {
	const char* json = "{\
		\"name\": \"How To Write A JSON Marshaller In C\",\
		\"content\": \"TODO\",\
		\"views\": null,\
		\"user\": {\
			\"uid\": 1000,\
			\"username\": \"overflowerror\",\
			\"aliases\": [\
				\"foo\",\
				\"bar\"\
			],\
			\"email\": \"overflow@persei.net\"\
		}\
	}";
	
	post_t* post = json_unmarshall(post_t, json);
	
	printf("Name:          %s\n", post->name);
	printf("Content:       %s\n", post->content);
	printf("Views:         %s (%ld)\n", post->views == NULL ? "null" : "", post->views == NULL ? 0 : *post->views);
	printf("User.Uid:      %lld\n", post->user.uid);
	printf("User.Username: %s\n", post->user.username);
	printf("User.Email:    %s\n", post->user.email);
	printf("User.Aliases:  ");
	for (size_t i = 0; post->user.aliases[i] != NULL; i++) {
		printf("%s ", post->user.aliases[i]);
	}	
	printf("\n\n");
	
	free(post->content);
	post->content = "Just do it.";
	
	json_free_prim_array(post->user.aliases);
	post->user.aliases = alloca(sizeof(char*) * 3);
	post->user.aliases[0] = "overflowerror";
	post->user.aliases[1] = "overflow";
	post->user.aliases[2] = NULL;
	
	char* newJson = json_marshall(post_t, post);
	printf("%s\n", newJson);
	free(newJson);
	
	// set to NULL so it doesn't get freed
	post->content = NULL;
	post->user.aliases = NULL;
	json_free_struct(post_t, post);

	return 0;
}
