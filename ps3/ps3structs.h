#ifndef H_PS3STRUCTS
#define H_PS3STRUCTS

#include <stdlib.h>

typedef struct
{
	void *(*malloc)(size_t s);
	void (*free)(void* ptr);
	void *(*realloc)(void* ptr, size_t s);
	int (*strcmp)(const char* s1, const char* s2);
	void* (*memcpy)(void* s1, const void* s2, size_t n);
	size_t (*strlen)(const char *s);
	
} ps3std_t;

typedef struct 
{
	ps3std_t* stds;
	void* exports;
} package_t;

#endif