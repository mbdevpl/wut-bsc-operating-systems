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
#include <ctype.h>

#define SIZE_HEAD sizeof(packet)
#define SIZE_BODY (PIPE_BUF - SIZE_HEAD)

struct packet {
	int pid; //of client
	char filename[32];
	int pos;
	ssize_t n;
};

typedef struct packet packet;

size_t bulk_read(int fd, char *buf, size_t count);
size_t bulk_write(int fd, char *buf, size_t count, int delay);
void makefifo(char* arg);
void closefifo(int fifo);
void closefile(int file);
void unlinkfifo(char* arg);

size_t bulk_read(int fd, char *buf, size_t count) {
	int c;
	size_t len=0;
	do{
		c=TEMP_FAILURE_RETRY(read(fd,buf,count));
		if(c<0) return c;
		if(c==0) return len; //EOF
		buf+=c;
		len+=c;
		count-=c;
	}while(count>0);
	return len ;
}

size_t bulk_write(int fd, char *buf, size_t count, int delay) {
	int c;
	size_t len=0;
	do{
		c=TEMP_FAILURE_RETRY(write(fd,buf,count));
		if(c<0) return c;
		buf+=c;
		len+=c;
		count-=c;
		sleep(delay);
	}while(count>0);
	return len ;
}

void makefifo(char* arg) {
	if(mkfifo(arg, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP)<0)
		if(errno!=EEXIST){
			perror("Create fifo:");
			exit(EXIT_FAILURE);
		}
}

void closefifo(int fifo) {
	if(TEMP_FAILURE_RETRY(close(fifo))<0){
			perror("Close fifo:");
			exit(EXIT_FAILURE);
	}
}

void closefile(int file) {
	if(TEMP_FAILURE_RETRY(close(file))<0){
			perror("Close file:");
			exit(EXIT_FAILURE);
	}
}

void unlinkfifo(char* arg) {
	if(unlink(arg)<0){
			perror("Remove fifo:");
			exit(EXIT_FAILURE);
	}
}

