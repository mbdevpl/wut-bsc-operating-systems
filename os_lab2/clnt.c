#include "clntserv.h"

/*
int pid; //of client
char[32] filename;
int pos;
int n;
*/

void usage(void){
	fprintf(stderr,"USAGE: clnt fifo_file delay svname [clname]\n");
}

int write_to_fifo(int fifo, int file, char* svname, int delay){
	//ssize_t count;
	char buffer[PIPE_BUF];
	char *buf;
	int total_read = 0;
	packet header;
	int total_packets = 0;
	
	header.pid = getpid();
	strcpy(header.filename, svname);
	//header.n = bulk_read(file, buf, SIZE_BODY);
	
	*((packet *)buffer) = header;
	buf = buffer + SIZE_HEAD;
	
	do{
		header.n = bulk_read(file, buf, SIZE_BODY);
		header.pos = total_read;
		total_read += header.n;
		
		fprintf(stdout, "sending packet no.%d (len = %d)...\n",
			total_packets + 1, header.n);
		
		if(header.n < 0){
			perror("Read:");
			exit(EXIT_FAILURE);
		}
		//fprintf(stdout, "O.K.\n");
		if(header.n < SIZE_BODY)
			memset(buf + header.n, 0, SIZE_BODY - header.n);
		fprintf(stdout, "O.K.\n");
		if(header.n > 0){
			if(bulk_write(fifo,buffer,PIPE_BUF,delay)<0){
				perror("Write:");
				exit(EXIT_FAILURE);
			}
		}
		//fprintf(stdout, "O.K.\n");
		total_packets ++;
		fprintf(stdout, "sent packet no.%d...\n", total_packets);
	}while(header.n == SIZE_BODY);
	
	return total_packets;
}

int main(int argc, char** argv) {
	int fifo, file;
	int delay;
	int sent_packets;
	
	fprintf(stdout, "pid = %d\n", getpid());
	
	if(argc > 5 || argc < 4) {
		usage();
		return EXIT_FAILURE;
	}
	
	makefifo(argv[1]);
	
	if((fifo=TEMP_FAILURE_RETRY(open(argv[1],O_WRONLY)))<0){
			perror("Open fifo:");
			exit(EXIT_FAILURE);
	}
	
	delay = atoi(argv[2]);
	
	if(argc == 5) {
		if((file=TEMP_FAILURE_RETRY(open(argv[4],O_RDONLY)))<0){
				perror("Open file:");
				exit(EXIT_FAILURE);
		}
		sent_packets = write_to_fifo(fifo, file, argv[3], delay);
		
		fprintf(stdout, "sent packets: %d\n", sent_packets);
	} else {
		//user input, not implemented
		sent_packets = 0;
		fprintf(stderr, "ERROR: user input not implemented!\n");
		return EXIT_FAILURE;
	}
	
	if(argc == 5)
		closefile(file);
	closefifo(fifo);
	
	
	return EXIT_SUCCESS;
}

