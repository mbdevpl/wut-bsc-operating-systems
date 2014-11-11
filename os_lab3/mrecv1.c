/* mrecv.c - a simple message reader.
 * An example of  UNIX System V IPC message queue programming.
 * The author: M. Borkowski.
 */
#define _GNU_SOURCE 
#include <unistd.h>
#ifndef TEMP_FAILURE_RETRY
#define TEMP_FAILURE_RETRY(expression) \
  (({ long int __result;                                                     \
       do __result = (long int) (expression);                                 \
       while (__result == -1L && errno == EINTR);                             \
       __result; }))
#endif
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include "msendrcv.h"

int main(int argc, char** argv) {
	int queue;
	packet p1;
	if((queue = msgget(QUEKEY,IPC_CREAT|S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP))<0){
		perror("Create queue:");
		exit(EXIT_FAILURE);
	}
			
	for(;;){
		if(TEMP_FAILURE_RETRY(msgrcv(queue,&p1,PACKET,1,0))<0) break;
		printf("%s",p1.mtext);
	}
	if(errno) perror("mrecv1 error");
	return EXIT_SUCCESS;
}
