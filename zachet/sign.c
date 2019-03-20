#include <stdio.h>
#include <unistd.h>
#include <signal.h>

int check = 0;
int count = 0;
int finish = 0;
int pfd[2];

void handler(int sig){
    if(check == count)
        finish = 1;
}

int main(){
    pipe(pfd);

    struct sigaction act = {};
    act.sa_handler = handler;
    sigaction(SIGALRM, &act, NULL);

    char buf[256] = {};
    while(1){
        check = count;
        alarm(1);
        int res = write(pfd[1], buf, 256);
        if(finish)
            break;
        count += res;
    }
    printf("%d\n", count);
    return 0;
}
