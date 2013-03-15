#include "clntserv.h"

void usage(void){
	fprintf(stderr,"USAGE: serv fifo_file\n");
}


size_t filter_buffer(char* buf, int len) {
	size_t i,j;
	for(i=j=0;i<len;i++)
		if(isalnum(buf[i])) buf[j++]=buf[i];
	return j;
}

void append_to_file (char *filename,char *buf, size_t len){
	int fd;
	if((fd=TEMP_FAILURE_RETRY(open(filename,O_WRONLY|O_APPEND|O_CREAT,
				S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP)))<0){
		perror("Open file:");
		exit(EXIT_FAILURE);
	}
	if(bulk_write(fd,buf,len,0)<0){
		perror("Write:");
		exit(EXIT_FAILURE);
	}
	if(TEMP_FAILURE_RETRY(close(fd))<0){
		perror("Close file:");
		exit(EXIT_FAILURE);
	}
}

void read_from_fifo(int fifo){
	ssize_t count, i;
	char buffer[PIPE_BUF];
	char filehead[128];
	packet header;
	
	do{
		count=bulk_read(fifo,buffer,PIPE_BUF);
		if(count<0){
			perror("Read:");
			exit(EXIT_FAILURE);
		}
		fprintf(stdout, "O.K.\n");
		if(count>0){
			header = *((packet*)buffer);
			snprintf(filehead,128,"pid=%d filename=%s pos=%d n=%d\n",
				header.pid, header.filename, header.pos, header.n);
			
			i=filter_buffer(buffer+sizeof(packet),SIZE_BODY);
			append_to_file(header.filename,buffer+sizeof(packet),i);
		}
	}while(count==PIPE_BUF);
}

int main(int argc, char** argv) {
	int fifo;
	if(argc != 2) {
		usage();
		return EXIT_FAILURE;
	}
	
	makefifo(argv[1]);
	
	if((fifo=TEMP_FAILURE_RETRY(open(argv[1],O_RDONLY)))<0){
			perror("Open fifo:");
			exit(EXIT_FAILURE);
	}
	read_from_fifo(fifo);
	
	closefifo(fifo);
	unlinkfifo(argv[1]);
	
	return EXIT_SUCCESS;
}

