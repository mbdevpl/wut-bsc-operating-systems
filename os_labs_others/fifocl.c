/* fifocl.c - the program copies lines of text from standard input
 * to the FIFO (named MYFIFO). See also fifosv.c .
 */
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

#define FIFO_FILE       "MYFIFO"
void error(char *message)
{
	perror(message);
	kill(0, SIGTERM);
	exit(EXIT_FAILURE);
}

void setsighandler(int sig, void (*handler) (int))
{
	struct sigaction sa;

	memset(&sa, 0, sizeof(struct sigaction));
	sa.sa_handler = handler;
	if (sigaction(sig, &sa, NULL) == -1)
		error("sigaction");
}

void handler(int sig)
{
	fputs("SIGPIPE\n", stderr);
	return;
}

int main(int argc, char *argv[])
{

	char buf[80];
	int fd, m, n;

	setsighandler(SIGPIPE, handler);
	if ((fd = open(FIFO_FILE, O_WRONLY)) < 0) {
		perror("open");
		return (1);
	}
	fputs("FIFO opened\n", stderr);
	while (1) {
		while ((n = read(0, buf, sizeof(buf))) > 0) {
			if ((m = write(fd, buf, n)) != n) {
				fprintf(stderr, "m=%d, n=%d\n", m, n);
				perror("fifocl:write");
				return 2;
			} else {
				fprintf(stderr, "%d bytes sent to FIFO (%s)\n",
					m, FIFO_FILE);
			}
		}
		if (n == 0) {
			fputs("EOD\n", stderr);
			return 0;
		}
		if (errno != EINTR)
			break;
		fputs("EINTR\n", stderr);
	}
	perror("fifocl.c");
	return 3;
}
