#ifndef H_PS3LIB
#define H_PS3LIB

#include "ps3structs.h"


void* malloc(size_t s);
void  free(void* ptr);
void* realloc(void* ptr, size_t s);
int strcmp(const char* s1, const char* s2);
void* memcpy(void* s1, const void* s2, size_t n);
size_t strlen(const char *s);
DIR *opendir(const char* name);
int closedir(DIR* dirp);
struct dirent* readdir(DIR* dirp);
void* memset(void*s, int c, size_t n);
int strcasecmp(const char *a, const char *b);
int stat(const char *path, struct stat *buf);

#endif