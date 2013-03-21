#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

int main(int argc, char** argv) {
	int children, timeout, m;
	
	if(argc != 4)
		return usage();
	
	children = atoi(argv[1]);
	timeout = atoi(argv[3]);
	m = atoi(argv[2]);
	
	if(children < 1 || timeout < 1 || m < 1)
		return usage();
	
	return EXIT_SUCCESS;
}

int usage() {
	fprintf(stderr,"You have to enter 4 arguments:\n 1) number of children > 0\n 2) ?\n 3) timeout >= 1\n");
	return EXIT_FAILURE;
}
