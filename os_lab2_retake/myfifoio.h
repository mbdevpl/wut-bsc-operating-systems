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

void makefifo(char* arg);
void closefifo(int fifo);
void unlinkfifo(char* arg);

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

void unlinkfifo(char* arg) {
	if(unlink(arg)<0){
			perror("Remove fifo:");
			exit(EXIT_FAILURE);
	}
}
