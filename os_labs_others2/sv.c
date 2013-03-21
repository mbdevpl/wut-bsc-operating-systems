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
#include <clsv.h>
#define MSG_SIZE (PIPE_BUF - sizeof(pid_t) - sizeof(fname) - sizeof(off_t) - sizeof(size_t))
void usage(void)
{
	fprintf(stderr,"USAGE: server fifo_file timeout\n");
}
void pipe_handler(int sig) 
{
	printf("[%d] received SIGPIPE and must terminate.\n", getpid());
}
void append_to_file (struct packet *ptr)
{
	int fd;
	ssize_t len = ptr.length;
	if((fd=TEMP_FAILURE_RETRY(open(ptr.fname,O_WRONLY|O_APPEND|O_CREAT,S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP)))<0)
	{
			perror("Open file:");
			exit(EXIT_FAILURE);
	}
	if(bulk_write(fd,ptr,len)<0)
	{
		perror("Write:");
		exit(EXIT_FAILURE);
	}
	if(TEMP_FAILURE_RETRY(close(fd))<0){
			perror("Close file:");
			exit(EXIT_FAILURE);
	}
}

void read_from_fifo(int fifo, int timeout)
{
	ssize_t count;
	char buffer[PIPE_BUF];
	struct packet *ptr; 
	do{
		count=bulk_read(fifo,buffer,PIPE_BUF);
		ptr = malloc(sizeof(count));
		if(count<0)
		{
			perror("Read:");
			exit(EXIT_FAILURE);
		}
		if(count>0)
		{
			snprintf(fname,32,"%d.txt",ptr.fname);
			append_to_file(ptr);
		}
	}while(count==PIPE_BUF);
}

int main(int argc, char** argv) 
{
	int fifo int timeout;
	if(argc=3) {
		usage();
		return EXIT_FAILURE;
	}
	int timeout = atoi(argv[2]);
	if(mkfifo(argv[1], S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP)<0)
		if(errno!=EEXIST)
		{
			perror("Create fifo:");
			exit(EXIT_FAILURE);
		}
	if((fifo=TEMP_FAILURE_RETRY(open(argv[1],O_RDONLY)))<0)
	{
			perror("Open fifo:");
			exit(EXIT_FAILURE);
	}
	if(sethandler(pipe_handler,SIGPIPE)) 
	{
		perror("Seting SIGPIPE:");
		exit(EXIT_FAILURE);
	}
	read_from_fifo(fifo,timeout);	
	if(TEMP_FAILURE_RETRY(close(fifo))<0)
	{
			perror("Close fifo:");
			exit(EXIT_FAILURE);
	}
	if(unlink(argv[1])<0)
	{
			perror("Remove fifo:");
			exit(EXIT_FAILURE);
	}
	return EXIT_SUCCESS;
}

