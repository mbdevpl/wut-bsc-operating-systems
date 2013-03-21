/* pgrp.c - a program, that creates n (program parameter) sub-processes.
 * Some children processes (selected at random) become leaderes of
 * a new process group. After some delay the main sends SIGINT to the
 * group, to which it belongs. Recepients of the signals display
 * a message upon SIGINT delivery and finish.
 * Note, that the new leaders do not get the signal and finish later,
 * after receiving SIGALRM signal.
*/
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <sys/times.h>
#include <unistd.h>
#include <wait.h>
volatile sig_atomic_t sig_flag = 0;

void error(char *message)
{
	perror(message);
	kill(0, SIGTERM);
	exit(EXIT_FAILURE);
}

void setsighandler(int sig, void (*handler) (int))
{
	struct sigaction sa;

	memset(&sa, 0x00, sizeof(struct sigaction));
	sa.sa_handler = handler;
	if (sigaction(sig, &sa, NULL) == -1)
		error("sigaction");
}

void sighandler(int sig)
{
	printf("signal %d handler for PID = %lu\n",
	       sig, (unsigned long)getpid());
	sig_flag++;
}

int getrand(int lo, int hi)
{				/* get a random integer from the interval [lo,hi] */
	double A = (double)(hi - lo + 1);
	return lo + (int)((rand() * A) / (RAND_MAX + 1.0));
}

void usage(char *prg)
{
	fprintf(stderr, "%s usage: pgrp  n\n", prg);
	fprintf(stderr, "pgrp creates n subprocesses, and then\n");
	fprintf(stderr, "sends SIGINT signal to the process group\n");
	fprintf(stderr, "to which the main process and its children belong\n");
	exit(1);
}

int main(int argc, char *argv[])
{
	int n, ni, status;
	pid_t pid;
	if (argc <= 1)
		usage(argv[0]);
	n = atoi(argv[1]);
	if (n <= 0)
		usage(argv[0]);
	setsighandler(SIGINT, sighandler);
	setsighandler(SIGALRM, sighandler);
	srand((unsigned int)times(NULL));
	printf("main: PID=%lu, PPID=%lu, PGRP=%lu\n", (unsigned long)getpid(),
	       (unsigned long)getppid(), (unsigned long)getpgrp());
	fflush(NULL);
	for (ni = 0; ni < n; ni++) {
		int flag;
		flag = getrand(0, 1);
		switch (fork()) {
		case -1:
			perror("fork");
			exit(2);
		case 0:	/* child process */
			if (flag) {
				if (setpgid(0, 0))
					perror("setpgrp");
			}
			alarm(10);	/* just to limit lifetime of the process */
			printf("started PID=%lu, PPID=%lu, PGRP=%lu\n",
			       (unsigned long)getpid(),
			       (unsigned long)getppid(),
			       (unsigned long)getpgrp());
			fflush(NULL);
			do {
				pause();
			} while (!sig_flag);
			printf("exiting, PID=%lu\n", (unsigned long)getpid());
			exit(0);
			break;
		default:
			break;
		}
		sleep(1);
	}
	/* sending SIGINT signal to the process group */
	if (kill(0, SIGINT))
		perror("kill");	/* of the main process */
	alarm(20);		/* set SIGALRM, to limit waiting time in the while below */
	while (!sig_flag)
		sleep(1);	/* waiting for a signal */
	while ((pid = waitpid(0, &status, WNOHANG)) > 0) {
		if (WIFEXITED(status)) {
			printf("Process nr %lu exited with code=%d\n",
			       (unsigned long)pid, WEXITSTATUS(status));
		}
		if (WIFSIGNALED(status)) {
			printf("Process nr %lu got signal nr %d\n",
			       (unsigned long)pid, WTERMSIG(status));
		}
		if (WIFSTOPPED(status)) {
			printf("Process nr %lu stopped by signal nr %d\n",
			       (unsigned long)pid, WSTOPSIG(status));
		}
	}
	printf("Main exiting\n");
	return 0;
}
