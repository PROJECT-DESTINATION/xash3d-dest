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