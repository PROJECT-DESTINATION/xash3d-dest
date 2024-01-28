#include <sys/syscall.h>
#include <sys/socket.h>


int socket(int family ,int type, int protocol)
{
	system_call_3(713,family,type,protocol);
	return_to_user_prog(int);
}

int bind(int s, const struct sockaddr *addr, socklen_t addrlen)
{
	system_call_3(701,s,addr,addrlen);
	return_to_user_prog(int);
}

int setsockopt(int s, int level, int optname, const void *optval, socklen_t optlen)
{
	system_call_5(711,s,level,optname,optval,optlen);
	return_to_user_prog(int);
}

int recvfrom(int s, void *buf, size_t len, int flags, struct sockaddr *addr, socklen_t *paddrlen)
{
	system_call_6(707,s,buf,len,flags,addr,paddrlen);
	return_to_user_prog(int);
}

int sendto(int s, const void *buf, size_t len, int flags, const struct sockaddr *addr, socklen_t addrlen)
{
	system_call_6(710,s,buf,len,flags,addr,addrlen);
	return_to_user_prog(int);
}