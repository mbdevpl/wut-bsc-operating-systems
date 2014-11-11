#define _GNU_SOURCE
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <limits.h>
#include <ctype.h>
#define MSG_SIZE (PIPE_BUF - sizeof(pid_t) - sizeof(fname) - sizeof(off_t) - sizeof(ssize_t))
struct packet
{
	pid_t pid;
	char fname[32];
	off_t pos;
	ssize_t length;
	char content[MSG_SIZE];
}

size_t bulk_read(int fd, struct packet *ptr, size_t count)
{
	int c;
	size_t len=0;
	do
	{
		c=TEMP_FAILURE_RETRY(read(fd,ptr,count));
		if(c<0) 
			return c;
		if(c==0) 
			return len; //EOF
		buf+=c;
		len+=c;
		count-=c;
	}while(count>0);
	return len;
}

size_t bulk_write(int fd, struct packet *ptr, size_t count)
{
	int c;
	size_t len=0;
	do
	{
		c=TEMP_FAILURE_RETRY(write(fd,ptr,count));
		if(c<0) 
			return c;
		buf+=c;
		len+=c;
		count-=c;
	}while(count>0);
	return len;
}
int sethandler( void (*f)(int), int sigNo) 
{
	struct sigaction act;
	memset(&act, 0, sizeof(struct sigaction));
	act.sa_handler = f;
	if (-1==sigaction(sigNo, &act, NULL))
		return -1;
	return 0;
}
