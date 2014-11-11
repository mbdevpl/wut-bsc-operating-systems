#define _GNU_SOURCE
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <limits.h>
#include <signal.h>
#include <string.h>
#include <clsv.h>
#define MSG_SIZE (PIPE_BUF - sizeof(pid_t) - sizeof(fname) - sizeof(off_t) - sizeof(size_t))
void usage(void)
{
	fprintf(stderr,"USAGE: client fifo_file timeout server_name client_name \n");
}

void pipe_handler(int sig) 
{
	printf("[%d] received SIGPIPE and must terminate.\n", getpid());
}

void write_to_fifo(int fifo, int file, int timeout, char *svname, char *clname)
{
	ssize_t count;
	//char buffer[PIPE_BUF];
	//char *buf;
	//*((pid_t *)buffer)=getpid();
	//buf=buffer+sizeof(pid_t);
	struct packet *ptr;
	ptr.pid = getpid();
	if(clname!=NULL)
		ptr.fname = clname;
	else 
	 	ptr.fname = NULL;
	ptr.pos = lseek(file,0,SEEK_CUR);
	ptr.length = MSG_SIZE;
	do{
		count=bulk_read(file,ptr,MSG_SIZE);
		if(count<0)
		{
			perror("Read:");
			exit(EXIT_FAILURE);
		}
		if(count>0)
		{
			if(bulk_write(fifo,ptr,MSG_SIZE)<0)
			{
				perror("Write:");
				exit(EXIT_FAILURE);
			}
		}
		sleep(1);
		if(count == PIPE_BUF)
		{
			sleep(timeout);
			return;
		}
	}while(count==MSG_SIZE);
}

int main(int argc, char** argv) 
{
	int fifo, file;
	if(argc > 5) 
	{
		usage();
		return EXIT_FAILURE;
	}
	int timeout = atoi(argv[2]);
	char *svname = malloc(sizeof(argv[3]));
	char *clname = malloc(sizeof(argv[3]));
	svname = argv[3];
	clname = argv[4]; 
	if(mkfifo(argv[1], S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP)<0)
		if(errno!=EEXIST)
		{
			perror("Create fifo:");
			exit(EXIT_FAILURE);
		}
	if((fifo=TEMP_FAILURE_RETRY(open(argv[1],O_WRONLY)))<0)
	{
			perror("Open fifo:");
			exit(EXIT_FAILURE);
	}
	if((file=TEMP_FAILURE_RETRY(open(argv[4],O_RDONLY)))<0)
	{
			perror("Open file:");
			exit(EXIT_FAILURE);
	}
	if(sethandler(pipe_handler,SIGPIPE)) 
	{
		perror("Seting SIGPIPE:");
		exit(EXIT_FAILURE);
	}
	write_to_fifo(fifo,file,timeout,svname,clname);	
	if(TEMP_FAILURE_RETRY(close(file))<0)
	{
			perror("Close file:");
			exit(EXIT_FAILURE);
	}
	if(TEMP_FAILURE_RETRY(close(fifo))<0)
	{
			perror("Close fifo:");
			exit(EXIT_FAILURE);
	}
	return EXIT_SUCCESS;
}
