/* prg1.c */
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wait.h>
#define	Nmax	10
volatile sig_atomic_t sigint_cnt = 0, sig_nr = 0, child_end = 0;
pid_t PID[Nmax];
int child_nr, M = 3, N, USR1_cnt, USR2_cnt;
// comment out the following line, to remove print-outs from signal handlers
//#define DBG
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

void setNOINThandler(int sig, void (*handler) (int))
{
	struct sigaction sa;

	memset(&sa, 0x00, sizeof(struct sigaction));
	sigaddset(&(sa.sa_mask), SIGINT);	/* blocking SIGINT */
	sa.sa_handler = handler;
	if (sigaction(sig, &sa, NULL) == -1)
		error("sigaction");
}

void ALRM_handler(int sig)
{
	sig_nr = sigint_cnt;
	sigint_cnt = 0;
#ifdef DBG
	printf("%d signal handler, PID = %lu\n", sig, (unsigned long)getpid());
#endif
	if (sig_nr > 1)
		sig_nr = SIGUSR2;
	else
		sig_nr = SIGUSR1;
#ifdef DBG
	printf("kill -> PID=%lu,sig_nr=%d\n", (unsigned long)PID[child_nr],
	       sig_nr);
#endif
	if (kill(PID[child_nr], sig_nr))
		perror("kill");
	child_nr++;
	child_nr = child_nr % N;
}

void INT_handler(int sig)
{
#ifdef DBG
	printf("SIGINT handler, PID = %lu, sigint_cnt=%d\n",
	       (unsigned long)getpid(), sigint_cnt);
#endif
	if (!sigint_cnt)
		alarm(1);
	sigint_cnt++;
}

void CHLD_handler(int sig)
{
	return;
}

void USR_handler(int sig)
{
	if (sig == SIGUSR1)
		USR1_cnt++;
	else if (sig == SIGUSR2)
		USR2_cnt++;
#ifdef DBG
	printf("%d signal handler, PID = %lu, USR1_cnt=%d, USR2_cnt=%d, M=%d\n",
	       sig, (unsigned long)getpid(), USR1_cnt, USR2_cnt, M);
#endif
	if (USR2_cnt >= M) {
		child_end = 1;
	}
}

int child_work(int nr)
{
	setsighandler(SIGINT, SIG_IGN);
	setsighandler(SIGUSR1, USR_handler);
	setsighandler(SIGUSR2, USR_handler);
	printf("child nr %d, PID=%lu\n", nr, (unsigned long)getpid());
	while (!child_end) {
		pause();
		printf("child nr %d, PID=%lu, USR1_cnt=%d, USR2_cnt=%d\n",
		       nr, (unsigned long)getpid(), USR1_cnt, USR2_cnt);

	}
	printf("child end, PID=%lu, USR1_cnt=%d\n", (unsigned long)getpid(),
	       USR1_cnt);
	return USR1_cnt;
}

void parent_work()
{
	int child_cnt = 0;
	setsighandler(SIGINT, INT_handler);
	setNOINThandler(SIGALRM, ALRM_handler);
	setsighandler(SIGCHLD, CHLD_handler);
/* Guess, what are consequences of uncommenting the following line ... */
	//setsighandler(SIGTERM, SIG_IGN);
	while (1) {
		pid_t pid;
		int status;
		pause();	/* wait for return from a signal handler */
		while ((pid = waitpid(0, &status, WNOHANG)) > 0) {
			child_cnt++;
			printf("waitpid: PID=%lu, ", (unsigned long)pid);
			if (WIFEXITED(status)) {
				printf("exit code=%d\n", WEXITSTATUS(status));
			} else {
				printf("status=%d\n", status);
			}
		}
		if (child_cnt) {
			kill(0, SIGTERM);
			break;
		}
	}
	return;
}

void usage(char *prg)
{
	fprintf(stderr, "%s usage: prg1 n m\n", prg);
	fprintf(stderr, "n - number of child processes (n<%d)\n", Nmax);
	fprintf(stderr, "m - nr of SIGUSR2 signals to end child work (def.: %d)\n", M);
	exit(1);
}

int main(int argc, char *argv[])
{
	int ni;
	if (argc <= 1)
		usage(argv[0]);
	N = atoi(argv[1]);
	if (N < 1 || N > Nmax)
		usage(argv[0]);
	if (argc > 2) {
		M = atoi(argv[2]);
		if (M < 1)
			usage(argv[0]);
	}
	printf("main process PID=%lu\n", (unsigned long)getpid());
	for (ni = 0; ni < N; ni++) {
		if ((PID[ni] = fork()) == -1) {
			perror("fork");
			exit(2);
		}
		if (PID[ni] == 0) {
			exit(child_work(ni + 1));
		}
	}
	parent_work();
	return 0;
}
