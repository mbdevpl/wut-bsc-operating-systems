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

//#include <stdio.h>
//#include <unistd.h>
//#include <stdlib.h>
#include <sys/sem.h>
#if defined(__GNU_LIBRARY__) && !defined(_SEM_SEMUN_UNDEFINED)
 /* union semun is defined in <sys/sem.h> */
#else
union semun{ int val; struct semid_ds *buf; unsigned short *array;};
#endif
int s_init(char *name, int cnt) {
	/* creation/attachment and initalization (to cnt) of a single semaphore */
	int semid;
	union semun arg;
	semid = semget(ftok(name, 1), 1, IPC_CREAT | 0600);
	arg.val=cnt;
	if(semctl(semid,0,SETVAL,arg)<0){   perror("sem_init");   exit(1); }
	return semid;
}

void s_wait(int semid) {/* wait operation */
	struct sembuf s;
	s.sem_num = 0;
	s.sem_op = -1;
	s.sem_flg = SEM_UNDO;
	if(semop(semid, &s, 1) < 0) { perror("sem_wait"); exit(1);}
}

void s_post(int semid) {/* post operation */
	struct sembuf s;
	s.sem_num = 0;
	s.sem_op = 1;
	s.sem_flg = SEM_UNDO;
	if(semop(semid, &s, 1) < 0) { perror("sem_post"); exit(1);}
}

void do_printing(int semid, char* fileName, char c) {
	//int i;
	FILE *fp;
	s_wait(semid); /* critical section: beginning */
	fp = fopen(fileName, "a+");
	fprintf(fp, "%c\n", c);
	fclose(fp);
	//for (i = 0; i < 5; i++) {
	   //printf("%c", c); fflush(stdout); sleep(1);
	//}
	s_post(semid); /* critical section: end */
	//sleep(1);
}

int main(int argc, char** argv) {

	int key = QUEKEY;
	
	int n = atoi(argv[2]);
	int total = n;
	
	int pid = getpid();
	int id;
	for(;n > 0; n--) {
		if(fork() == 0)
		{
			id = total - n + 1;
			printf("child no.%d pid = %d\n",id,getpid());
			break;
		}
	}
	if(pid == getpid()) {
		printf("created %d children\n",total);
		return EXIT_SUCCESS; 
	}
	
	int queue;
	struct msgbuf msg;
	if((queue = msgget(key,IPC_CREAT|S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP))<0){
		perror("Create queue:");
		exit(EXIT_FAILURE);
	}
	
	//reading
	int semidEven = s_init("evenFile",1);
	int semidOdd = s_init("oddFile",2);
	for(;;){
		//letters
		if(TEMP_FAILURE_RETRY(msgrcv(queue,&msg,1,(long)101,0))<0) break;
		char c = msg.mtext[0];
		printf("no.%d: %c\n", id,c);
		if(c >= 65 && c <= 90)
			do_printing(semidEven, "EVEN.txt", c);
		else
			do_printing(semidOdd, "odd.txt", c);
		
		//digits
		if(TEMP_FAILURE_RETRY(msgrcv(queue,&msg,1,(long)102,0))<0) break;
		c = msg.mtext[0];
		printf("no.%d: %d\n", id,c);
		if(c%2 == 0)
			do_printing(semidEven, "EVEN.txt", c);
		else
			do_printing(semidOdd, "odd.txt", c);
	}
	if(errno) perror("mrecv1 error");
	
	return EXIT_SUCCESS;
}

