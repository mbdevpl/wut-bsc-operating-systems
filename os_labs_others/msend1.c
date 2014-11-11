/* mrecv1.c - a simple message writer.
 * An example of  UNIX System V IPC message queue programming.
 * The author: M. Borkowski.
 */
#define _GNU_SOURCE 
#ifndef LINUX
#define TEMP_FAILURE_RETRY(expression) \
  (({ long int __result;                      \
       do __result = (long int) (expression);   \
       while (__result == -1L && errno == EINTR); \
       __result; }))
#endif
#include <unistd.h>
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
	
	int queue,i;
	packet p1;
	
	if((queue = msgget(QUEKEY,IPC_CREAT|S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP))<0){
		perror("Create queue:");
		exit(EXIT_FAILURE);
	}
	
	p1.mtype=1;	
	for(i = 0 ; i < 10; i++){
		snprintf(p1.mtext,PACKET,"Packet No %d\n",i);
		if(TEMP_FAILURE_RETRY(msgsnd(queue,&p1,PACKET,0))<0){
			perror("Write to queue:");
			exit(EXIT_FAILURE);
		}
		sleep(1);
	}
	fprintf(stderr,"Sleeping before exit()\n");
	sleep(5); /* Give the reader 5 sec for reading messages */	
	if(msgctl(queue,IPC_RMID,NULL)<0){
		perror("Remove queue:");
		exit(EXIT_FAILURE);
	}
	return EXIT_SUCCESS;
}
