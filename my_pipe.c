#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <ctype.h>
#include <limits.h>
#include <string.h>

int main(int argc, char* argv[]){
	int fd[2];
	int n;
	pid_t pid;
	int lines;
	int chars = 0;
	int words = 0;
	char buf[1];
	if(pipe(fd) < 0) {
		printf("Error with pipe\n");
		return 1;
	}
	if((pid = fork()) < 0){
		printf("error\n");
		return 2;
	}
	else if(pid > 0){/*parent*/
		close(fd[1]);
		while(read(fd[0], buf, 1) != 0) {
			chars++;
			if (isalnum(buf[0]) == 0)//not an alnum
			{
				if (buf[0] == '\n')
					lines++;
				else if (buf[0] != '\0')
					words++;
			}
		}
	}else{/*child*/
		close(1);
		dup(fd[1]);
		close(fd[1]);
		close(fd[0]);
		int er = execvp(argv[1], argv + 1);
		return 3;
	}
	printf("Chars: %d\nWords: %d\nLines: %d\n", chars, words, lines);
	return 0;
}

