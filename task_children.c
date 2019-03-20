#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>


int main(int argc, char* argv[]) {
	pid_t child_pid = 0;
	if (argc > 2) {
		for (int i = 1; i < argc; i++) {
			child_pid = fork();
			if (child_pid == -1) {
				printf("Error:no child created/n");
				return 1;
			}
			if (child_pid == 0) {
				char *endptr = 0;
				int num = strtol(argv[i], &endptr, 10);
				printf("pid = %d, ppid = %d\n", getpid(), getppid());
				printf("number = %d\n", num);
				return 0;
			}
			pid_t ret = 0;
			int wait_p = 0;
			ret = wait(&wait_p);
			if (ret == -1){
				printf("Error: incorrect wait\n");
				return 2;
			}
		}
	}

	return 0;
}
