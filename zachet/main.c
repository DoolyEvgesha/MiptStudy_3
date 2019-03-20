#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/wait.h>
#include <string.h>

int fd[2];
int fd1[2];

int main(int argc, char* argv[]) {
    int d = 0, r;

    pipe(fd);
    pid_t id = fork();

    if (id == 0) {
        close(fd[0]);
        write(fd1[1], &d, 4);

        do{
            d++;
            write(fd1[1],  &d, 4);
            write(fd[1], "c", 1);
            read(fd1[0], &r, 4);

        }while(1);

        printf("%d\n", d);

    } else {
        sleep(1);
        read(fd1[0], &r,4);
        printf("%d\n", r);

        close(fd[0]);
        close(fd[1]);
        close(fd1[0]);
        close(fd1[1]);
        wait(NULL);
    }

    return 0;
}