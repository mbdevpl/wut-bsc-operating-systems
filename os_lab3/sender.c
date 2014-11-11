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
#include "sendrcv.h"

int main(int argc, char** argv) {
	
	int key = QUEKEY;
	
	int queue, i;
	//packet p1;
	
	int even = 0, odd = 0, upper = 0, lower = 0;
	
	if((queue = msgget(key,IPC_CREAT|S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP))<0){
		perror("Create queue:");
		exit(EXIT_FAILURE);
	}
	
	srand(0);
	
	struct msgbuf msg1, msg2;
	for(i = 0 ; i < 60; i++){
		
		// letters
		
		msg1.mtype = 101;
		if(rand()%2) { // capitalization randomization
			upper++;
			char a = 65 + rand()%26;
			//msg1.mtext[0] = a;
			msg1.mtext[0] = a;
		} else {
			lower++;
			char a = 97 + rand()%26;
			//msg1.mtext = " ";
			msg1.mtext[0] = a;
		}
		
		//numbers
		
		msg2.mtype = 102;
		char i = rand()%100;
		msg2.mtext[0] = i;
		
		if(i%2) odd++;
		else even++;
	
		if(TEMP_FAILURE_RETRY(msgsnd(queue,&msg1,1,0))<0){
			perror("Write to queue:");
			exit(EXIT_FAILURE);
		}
		
		if(TEMP_FAILURE_RETRY(msgsnd(queue,&msg2,1,0))<0){
			perror("Write to queue:");
			exit(EXIT_FAILURE);
		}
		
		printf("sent: %c and %d\n", msg1.mtext[0], (int)msg2.mtext[0]);
		
		sleep(1);
	}
	
	printf("even  = %d\nodd   = %d\nupper = %d\nlower = %d\n", even, odd, upper, lower);
	
	if(msgctl(queue,IPC_RMID,NULL)<0){
		perror("Remove queue:");
		exit(EXIT_FAILURE);
	}
	
	return EXIT_SUCCESS;
}
