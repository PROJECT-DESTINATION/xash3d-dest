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
	char* (*strchr)(const char* s, int c);
	char* (*strncat)(char* a, const char* b, size_t n);
	int (*strncasecmp)(const char *a, const char *b, size_t i);
	void (*qsort)(void* base,size_t nmemb, size_t size, _Cmpfun* y);
	off_t (*lseek)(int filedes, off_t offset, int wh);
	int (*open)(const char* path, int oflag);
	int (*close)(int filedes);
	int (*read)(int filedes, void* buf, unsigned int nbyte);
	char* (*strncpy)(char *s1, const char* s2, size_t n);
	void* (*memmove)(void* s1, const void* s2, size_t n);
	long int (*strtol)(const char* nptr, char **endptr, int base);
	float (*_Stof)(const char* str, char** end, int what);
	int* (*_Geterrno)();
	char* (*strrchr)(const char *s, int c);
	char* (*strstr)(const char* s1, const char* s2);
	char* (*strerror)(int errnum);
	int (*mkdir)(const char* dir, mode_t mode);
	int (*write)(int filedes, const void* buf, unsigned int nbyte);
	char* (*strcpy)(char *s1, const char* s2);
	float (*_FSin)(float v, unsigned int l);
	
} ps3std_t;

typedef struct 
{
	ps3std_t* stds;
	void* exports;
} package_t;

#endif