/* tut1_3.c - child code enhancements */
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

void child_handler(int sig);
void child_work(int l);
int sethandler( void (*f)(int), int sigNo);
void sigchld_handler(int sig);
void create_children(int n, int r);
void usage(void);

volatile sig_atomic_t last_signal = 0;
/*************************************/
int sethandler( void (*f)(int), int sigNo) {
	struct sigaction act;
	memset(&act, 0, sizeof(struct sigaction));
	act.sa_handler = f;
	if (-1==sigaction(sigNo, &act, NULL))
		return -1;
	return 0;
}
/* child code */
void child_handler(int sig) {
	printf("[%d] received signal %d\n", getpid(), sig);
	last_signal = sig;
}
void child_work(int r) {
	int t,tt;

	srand(getpid());
	t = rand()%6+5; 

	while(r-- > 0){
		for(tt=t;tt>0;tt=sleep(tt));
		if (last_signal == SIGUSR1) printf("Success [%d]\n", getpid());
		else printf("Failed [%d]\n", getpid());
	}
	printf("[%d] Terminates \n",getpid());
}

/* parent code */
void sigchld_handler(int sig) {
	pid_t pid;
	for(;;){
		pid=waitpid(0, NULL, WNOHANG);
		if(pid==0) return;
		if(pid<=0) {
			if(errno==ECHILD) return;
			perror("waitpid:");
			exit(EXIT_FAILURE);
		}
	}
}
void create_children(int n, int r) {
	while (n-->0) {
		switch (fork()) {
			case 0:
				if(sethandler(child_handler,SIGUSR1)) {
					perror("Seting child SIGUSR1:");
					exit(EXIT_FAILURE);
				}
				if(sethandler(child_handler,SIGUSR2)) {
					perror("Seting child SIGUSR2:");
					exit(EXIT_FAILURE);
				}
				child_work(r);
				exit(EXIT_SUCCESS);

			case -1:
				perror("Fork:");
				exit(EXIT_FAILURE);

		}
	}
}
void usage(void){
	fprintf(stderr,"USAGE: signals n k p r\n");
	fprintf(stderr,"n - number of children\n");
	fprintf(stderr,"k - Interval before SIGUSR1\n");
	fprintf(stderr,"p - Interval before SIGUSR2\n");
	fprintf(stderr,"r - lifetime of child in cycles\n");
}

int main(int argc, char** argv) {
	int n, k, p, r;

	if(argc!=5) {
		usage();
		return EXIT_FAILURE;
	}

	n = atoi(argv[1]);
	k = atoi(argv[2]);
	p = atoi(argv[3]);
	r = atoi(argv[4]);

	if (n<=0 || k<=0 || p<=0 || r<=0) {
		usage();
		return EXIT_FAILURE;
	}
	if(sethandler(sigchld_handler,SIGCHLD)) {
		perror("Seting parent SIGCHLD:");
		exit(EXIT_FAILURE);
	}
	create_children(n, r);
	sleep(2);
	printf("Parent ready to exit\n");
	return EXIT_SUCCESS;
}