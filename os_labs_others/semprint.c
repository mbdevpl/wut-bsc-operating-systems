/* semprint.c - an example of IPC System V semaphore use. */
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
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
static int  flag;
void do_printing(int semid, char c) {
	int i;
	if(flag) s_wait(semid);
	for (i = 0; i < 5; i++) {/* critical section: beginning */
	   printf("%c", c); fflush(stdout); sleep(1);
	}/* critical section: end */
	if(flag) s_post(semid);
	sleep(1);
}
int main(int argc, char *argv[]){
	int semid;
	if(argc>1) flag=1;
	semid = s_init("semprint",1);
	if (fork() == 0) { // child process
		for (;;) do_printing(semid, '1');
		exit(0);
	}
	for (;;) do_printing(semid, '2');
	exit(0);
}
