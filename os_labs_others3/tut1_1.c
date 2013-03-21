/* tut1_1.c */
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

void create_children(int n, int l);
void usage(void);
/**********************************************/
void create_children(int n, int r) {
	while (n-->0) {
		switch (fork()) {
			case 0:
				printf("Created a child, PID=%d\n",getpid());
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

	create_children(n, r);
	sleep(2);
	printf("Parent ready to exit\n");
	return EXIT_SUCCESS;
}