#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>


int main(int argc, char* argv[]){
	pid_t child_pid = 0;
	pid_t wpid = 0;
	if(argc > 2){
		for(int i = 1; i < argc; i++) {
			child_pid = fork();
			if (child_pid == 0) {
				char *endptr = 0;
				int num = strtol(argv[i], &endptr, 10);
				printf("pid = %d, ppid = %d, i = %d\n", getpid(), getppid(), i);
				usleep(num*10000);
				printf("%d\n", num);
				return 0;
			}
		}
		pid_t ret;
		int wait_p;
		for(int j = 1; j < argc; j++) {
			ret = wait(&wait_p);
		}

		printf("\n");
	}
	return 0;
}
