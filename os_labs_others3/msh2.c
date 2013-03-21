/* msh2.c - a simple command interpreter.
 * Input can be redirected to a file (use: ./msh2 < commands_file).
 * There are 2 internal commands: ru, quit.
 * External commands are implemented as subprocesses running programs
 * specified as the first word of each command (PATH is taken into
 * account, to locate the programs in the file system).
 * Thus one cannot use usual shell tricks on commands, as
 * I/O redirection, piping, filename globbing, shell expansion,
 * background processing etc.
 */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wait.h>
/* Maximum nr of parameters in the command line */
#define MAXPAR	20

void print_rusage(FILE * out)
{				/* displays resource usage */
	double stime, utime;
	struct rusage ru;	/* see man getrusage for details */
	if (out == NULL)
		out = stderr;
	if (getrusage(RUSAGE_SELF, &ru) != -1) {
		stime =
		    (double)ru.ru_stime.tv_sec * 1e3 +
		    (double)ru.ru_stime.tv_usec * 1e-3;
		utime =
		    (double)ru.ru_utime.tv_sec * 1e3 +
		    (double)ru.ru_utime.tv_usec * 1e-3;
		fprintf(out, "\nParent   system mode time  = %10.3f ms\n",
			stime);
		fprintf(out, "Parent   user   mode time  = %10.3f ms\n", utime);
	}
	if (getrusage(RUSAGE_CHILDREN, &ru) != -1) {
		stime =
		    (double)ru.ru_stime.tv_sec * 1e3 +
		    (double)ru.ru_stime.tv_usec * 1e-3;
		utime =
		    (double)ru.ru_utime.tv_sec * 1e3 +
		    (double)ru.ru_utime.tv_usec * 1e-3;
		fprintf(out, "Children system mode time  = %10.3f ms\n", stime);
		fprintf(out, "Children user   mode time  = %10.3f ms\n", utime);
	}
	fflush(out);
	return;
}

int internal_cmd(char *buf)
{
	if (strncmp(buf, "ru", 6) == 0) {	/* display resource usage */
		print_rusage(NULL);
		return 1;
	} else if (strncmp(buf, "quit", 5) == 0) {	/* exit the interpreter */
		exit(0);
	}
	return 0;
}

int external_cmd(char *cmd)
{
	char *argv[MAXPAR + 1];
	int argc = 0, status;
	pid_t pid, pid2;
/* tokenization of the command line */
	if ((argv[argc] = strtok(cmd, " \t\n")) == NULL)
		return 0;
	while (argv[argc] && argc < MAXPAR - 1) {
		argv[++argc] = strtok(NULL, " \t\n");
	}
	argv[argc] = NULL;
	fflush(NULL);		/* flush all output buffers */
	if ((pid = fork()) < 0) {
		perror("fork");
		return -1;
	}
	if (pid == 0) {
		execvp(argv[0], argv);
		perror("execvp");
		exit(1);
	}
	while ((pid2=waitpid(0, &status, 0)) > 0) {
		if (WIFEXITED(status)) {
			printf("Process nr %lu exited with code=%d\n", (unsigned long)pid,
			       WEXITSTATUS(status));
		}
		if (WIFSIGNALED(status)) {
			printf("Process nr %lu got signal nr %d\n", (unsigned long)pid,
			       WTERMSIG(status));
		}
		if (WIFSTOPPED(status)) {
			printf("Process nr %lu stopped by signal nr %d\n",
			       (unsigned long)pid, WSTOPSIG(status));
		}
		if(pid2==pid) break;
	}
	sleep(1);
	fflush(NULL);		/* flush all output buffers */
	return 0;
}

int main(int argc, char *argv[])
{
	char buf[BUFSIZ];
	int status;
	fprintf(stderr, "%s: PID=%lu, PPID=%lu, UID=%lu, GID=%lu\n", argv[0],
		(unsigned long)getpid(), (unsigned long)getppid(),
		(unsigned long)getuid(), (unsigned long)getgid());
	while (1) {
		int cmdlen;
//              if(isatty(STDIN_FILENO))
		if (isatty(fileno(stdin)))
			printf("\n> ");	/* Display prompt if input comes from terminal */
		fflush(NULL);
		if (fgets(buf, sizeof(buf), stdin) == NULL)
			break;
		cmdlen = strlen(buf);
		if (buf[cmdlen - 1] == '\n')
			buf[cmdlen - 1] = '\0';
		if (buf[0] == '\0')
			continue;
		if (!isatty(STDIN_FILENO))	/* echo command if not input from terminal */
			printf("#cmd:# %s\n", buf);
		/* check for internal commands */
		if (internal_cmd(buf))
			continue;
		/* external commands are processed with a system shell */
		status = external_cmd(buf);
	}
	if (!feof(stdin)) {
		if (ferror(stdin)) {
			fprintf(stderr, "stdin error\n");
		}
	}
	print_rusage(NULL);
	return 0;
}
