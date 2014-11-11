#include "myfifoio.h"

int finished = 0;
int dataRead = 0;
int dataSent = 0;
int parentPid = -1;
int childPid = -1;

void usage(void){
	fprintf(stderr,"USAGE: path_to_output path_to_input timeout\n");
}

int sethandler( void (*handlerfunction)(int), int sigNo) {
	struct sigaction act;
	memset(&act, 0, sizeof(struct sigaction));
	act.sa_handler = handlerfunction;
	if (-1==sigaction(sigNo, &act, NULL))
		return -1;
	return 0;
}

void handleInt(int sig){
	//printf("[%d] received SIGINT\n", getpid());
	
	//finished = 1;
	if(parentPid > 0)
		printf("bytes_sent=%d\n", dataSent);
		
	if(childPid > 0)
		printf("bytes_received=%d\n", dataRead);
		
	sethandler(handleInt,SIGINT);
}

void handlePipe(int sig)
{
	printf("[%d] received SIGPIPE and must end writing\n", getpid());
	finished = 1;
}

int main(int argc, char** argv) {
	int fifoIn, fifoOut;
	int delay;
	
	if(argc != 4) {
		usage();
		return EXIT_FAILURE;
	}
	
	sethandler(handlePipe, SIGPIPE);
	sethandler(handleInt, SIGINT);
	
	//fifo initialisation
	makefifo(argv[1]);
	makefifo(argv[2]);
	delay = atoi(argv[3]);
	//printf("test\n");
	//printf("test\n");
	//printf("test\n");
	
	if(fork() > 0) {
		parentPid = getpid();
		if((fifoOut=TEMP_FAILURE_RETRY(open(argv[1],O_WRONLY)))<0){
				perror("Open fifo:");
				exit(EXIT_FAILURE);
		}
		printf("writer_pid=%d\n", parentPid);
		//printf("parent_pid=%d\n", getpid());
		//parent is writing to fifo
		do {
	
			char c;
			c = getchar();
			
			if(c == '\n') continue;
			if(c == EOF) finished = 1;
			if(finished) break;
			
			int count = TEMP_FAILURE_RETRY(write(fifoOut,&c,1));
			if(count <= 0) break;
			
			dataSent++;
			
		} while(1);
		finished = 1;
		closefifo(fifoOut);
		
		printf("bytes_sent=%d\n", dataSent);
		
	} else {
		if((fifoIn=TEMP_FAILURE_RETRY(open(argv[2],O_RDONLY)))<0){
				perror("Open fifo:");
				exit(EXIT_FAILURE);
		}
		childPid = getpid();
		printf("reader_pid=%d\n", childPid);
		//child is reading from fifo
		do {
			//sleep(delay);

			if(finished) break;
			
			char c;
			int count = TEMP_FAILURE_RETRY(read(fifoIn,&c,1));
			if(count <= 0) break;
			
			printf("char_received=%c\n",c);
			dataRead++;
			
		} while(1);
		finished = 1;
		closefifo(fifoIn);
		
		printf("bytes_received=%d\n", dataRead);
	}
	
	//fifo closing
	//closefifo(fifoIn);
	//closefifo(fifoOut);
	
	//printf("end\n");
	return EXIT_SUCCESS;
}
