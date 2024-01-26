#include "ps3lib.h"


extern ps3std_t* stds;

void* malloc(size_t s)
{
	return stds->malloc(s);
}

void  free(void* ptr)
{
	stds->free(ptr);
}

void* realloc(void* ptr, size_t s)
{
	return stds->realloc(ptr,s);
}

int strcmp(const char* s1, const char* s2)
{
	return stds->strcmp(s1,s2);
}

void* memcpy(void* s1, const void* s2, size_t n)
{
	return stds->memcpy(s1,s2,n);
}

size_t strlen(const char *s)
{
	return stds->strlen(s);
}

void* memset(void*s, int c, size_t n)
{
	return stds->memset(s,c,n);
}

DIR *opendir(const char* name)
{
	return stds->opendir(name);
}
int closedir(DIR* dirp)
{
	return stds->closedir(dirp);
}
struct dirent* readdir(DIR* dirp)
{
	return stds->readdir(dirp);
}
int strcasecmp(const char *a, const char *b)
{
	return stds->strcasecmp(a,b);
}
int stat(const char *path, struct stat *buf)
{
	return stds->stat(path,buf);
}

int vsnprintf(char *_Restrict A, size_t B,const char *_Restrict C, _Va_list D)
{
	return stds->vsnprintf(A,B,C,D);
}