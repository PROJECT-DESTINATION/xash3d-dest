#include "ps3lib.h"


extern ps3std_t* stds;

char curdir[260];

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

int chdir(const char* path)
{
	int l = strlen(path)+1;
	if(l > 260)
	{
		l = 260;
	}
	memcpy(curdir,path,l);
	curdir[259] = '\x00';
	return 0;
}

DIR *opendir(const char* name)
{
	if(name[0] == '/')
		return stds->opendir(name);
	char temp[260];
	temp[0] = '\x00';
	strncat(temp,curdir,260);
	strncat(temp,"/",260);
	strncat(temp,name,260);
	return stds->opendir(temp);
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
int strncasecmp(const char *a, const char *b, size_t i)
{
	return stds->strncasecmp(a,b,i);
}
int stat(const char *path, struct stat *buf)
{
	if(path[0] == '/')
		return stds->stat(path,buf);
	char temp[260];
	temp[0] = '\x00';
	strncat(temp,curdir,260);
	strncat(temp,"/",260);
	strncat(temp,path,260);
	return stds->stat(temp,buf);
}

int vsnprintf(char *_Restrict A, size_t B,const char *_Restrict C, _Va_list D)
{
	return stds->vsnprintf(A,B,C,D);
}

char* strchr(const char* s, int c)
{
	return stds->strchr(s,c);
}

char* strncat(char* a, const char* b, size_t n)
{
	return stds->strncat(a,b,n);
}
int open(const char* path, int oflag, ...)
{
	if(path[0] == '/')
		return stds->open(path,oflag);
	char temp[260];
	temp[0] = '\x00';
	strncat(temp,curdir,260);
	strncat(temp,"/",260);
	strncat(temp,path,260);
	return stds->open(temp,oflag);
}
void qsort(void* base,size_t nmemb, size_t size,_Cmpfun* y)
{
	return stds->qsort(base,nmemb,size,y);
}
off_t lseek(int filedes, off_t offset, int wh)
{
	return stds->lseek(filedes,offset,wh);
}
int close(int filedes)
{
	return stds->close(filedes);
}
int read(int filedes, void* buf, unsigned int nbyte)
{
	return stds->read(filedes,buf,nbyte);
}
int isdigit(int c)
{
	return c >= '0' && c <= '9';
}
char* strncpy(char *s1, const char* s2, size_t n)
{
	return stds->strncpy(s1,s2,n);
}

void* memmove(void* s1, const void* s2, size_t n)
{
	return stds->memmove(s1,s2,n);
}
long int strtol(const char* nptr, char **endptr, int base)
{
	return stds->strtol(nptr,endptr,base);
}

float _Stof(const char* str, char** end, int what)
{
	return stds->_Stof(str,end,what);
}
int *_Geterrno()
{
	return stds->_Geterrno();
}
char* strrchr(const char *s, int c)
{
	return stds->strrchr(s,c);
}
char* strstr(const char* s1, const char* s2)
{
	return stds->strstr(s1,s2);
}
char* strerror(int errnum)
{
	return stds->strerror(errnum);
}
int mkdir(const char* dir, mode_t mode)
{
	if(dir[0] == '/')
		return stds->mkdir(dir, mode);
	char temp[260];
	temp[0] = '\x00';
	strncat(temp,curdir,260);
	strncat(temp,"/",260);
	strncat(temp,dir,260);
	return stds->mkdir(temp, mode);
}
int write(int filedes, const void* buf, unsigned int nbyte)
{
	return stds->write(filedes,buf,nbyte);
}

char* strcpy(char *s1, const char* s2)
{
	return stds->strcpy(s1,s2);
}