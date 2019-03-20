#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#define size 1024

int main(int argc, char* argv){
	int fd[2], des, target, input;
	int b = -1;
	int bytes = -1;
	pid_t pid;
	char buf[size];
	void *mem, *data;
	if(pipe(fd) < 0){
		printf("Error: pipe doesn't work\n");
		return 1;
	}
	pid = fork();
	pid_t wait_p;
	if (pid == 0) {/* child write */
		des = open("input.txt", O_RDONLY);
        close(fd[0]);
		while(b != 0) {
			b = read(des, &buf, size);
			write(fd[1], buf, b);
		}
		close(des);
		close(fd[1]);
		return 0;
	} else {/* parent read */
	    	close(fd[1]);
	    	if((target = open("output.txt", O_CREAT|O_WRONLY|O_TRUNC, 0777)) == -1) {
				printf("Error: opening output.txt\n");
				return 1;
			}
		while((bytes = read(fd[0], &buf, size)) != 0) {
			printf("bytes = %d, length = %d\n", bytes, size);
			if(write(target, &buf, bytes) < 0){
				printf("Error: writing\n");
				return 2;
			}
		}
		close(target);
        	wait(&wait_p);
	}
	return 0;
}

