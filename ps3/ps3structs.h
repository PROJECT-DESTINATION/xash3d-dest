#ifndef H_PS3STRUCTS
#define H_PS3STRUCTS

#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>

typedef struct
{
	void *(*malloc)(size_t s);
	void (*free)(void* ptr);
	void *(*realloc)(void* ptr, size_t s);
	int (*strcmp)(const char* s1, const char* s2);
	void* (*memcpy)(void* s1, const void* s2, size_t n);
	size_t (*strlen)(const char *s);
	DIR *(*opendir)(const char* name);
	int (*closedir)(DIR* dirp);
	struct dirent* (*readdir)(DIR* dirp);
	void* (*memset)(void*s, int c, size_t n);
	int (*strcasecmp)(const char *a, const char *b);
	int (*stat)(const char *path, struct stat *buf);
	int (*vsnprintf)(char *_Restrict, size_t,const char *_Restrict, _Va_list);
	
} ps3std_t;

typedef struct 
{
	ps3std_t* stds;
	void* exports;
} package_t;

#endif