// Mateusz Bysiek, OS lab1
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

int children;
int procid;
int pidprinted;
int n;

int printed; //used only by 5th children
int timeout; //used only by children 1 to 4
int runtimer;

int childtokill;
int pids[6]; //process ids of children

int sethandler( void (*handlerfunction)(int), int sigNo) {
	struct sigaction act;
	memset(&act, 0, sizeof(struct sigaction));
	act.sa_handler = handlerfunction;
	if (-1==sigaction(sigNo, &act, NULL))
		return -1;
	return 0;
}

void handleChild(int sig){
	if(sig == SIGUSR1) { //signal to start timer
		runtimer = 1;
	} else if(sig == SIGALRM) { //alarm clock
		if(n == 5) {
			if(printed >= 10) {
				fprintf(stdout, "endPid=%childtokilld ", getpid());
				fprintf(stdout, "printed=%d ", printed);
				exit(EXIT_SUCCESS);
			}
		} else {
			if(timeout == 0) {
				fprintf(stdout, "endPid=%d ", getpid());
				fprintf(stdout, "printed=%d ", printed);
				exit(EXIT_SUCCESS);
			}
			if(runtimer)
				timeout--;
		}
	
		if(pidprinted) {
			printed++;
			fprintf(stderr, "%d", n);
		} else {
			pidprinted = 1;
			fprintf(stderr, " [pid=%d]\n", getpid());
		}
	}
}

void handleMain(int sig){
	if(sig == SIGUSR1 && children < 5) {
		children++;
		
		int pid = fork();
		if(pid == 0) {
			procid = getpid();
			pidprinted = 0;
			n = children;
			
			sethandler(handleChild, SIGALRM);
			sethandler(handleChild, SIGUSR1);
			
			printed = 0;
			runtimer = 0;
			timeout = n;
			int t;
			while(1) {
				for(t=1;t>0;t=sleep(t));
				raise(SIGALRM);
			}
		} else if(pid > 0) {
			pids[children-1] = pid;
			sethandler(handleMain, SIGUSR1);
		}
	}
}

void handleChildEnded(int sig){
	if(sig == SIGCHLD) {
		fprintf(stdout, "child DIED\n");
		fflush(stdout);
		kill(pids[childtokill], SIGUSR1); //start timer for next child that will
		//be killed off
		childtokill--;
	}
}
		

int main(int argc, char** argv) {
	children = 0;
	childtokill = 4;
	sethandler(handleMain, SIGUSR1);
	sethandler(handleChildEnded, SIGCHLD);
	fprintf(stdout, "PID:%d\n", getpid());
	
	int t;
	char c;
	while(1) {
		for(t=1;t>0;t=sleep(t));
		//if()
		
		/*c=getchar();
		if(c == 'u') {
			raise(SIGUSR1);
		}*/
	}
}

