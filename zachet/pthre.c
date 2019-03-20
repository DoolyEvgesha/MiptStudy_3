#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>

int pfd[2];
int count = 0;
pthread_t thrid[2];

void* first(void* arg){
    char buf[256] = {};
    while(1){
        int res = write(pfd[1], buf, 256);
        if(res == -1)
            break;
        count += res;
    }
}

void* second(void* arg){
    while(1){
        int first = count;
        sleep(1);
        int second = count;
        if(first == second)
            break;
    }
    printf("%d\n", count);
    close(pfd[0]);
}


int main(){
    pipe(pfd);
    pthread_create(thrid, NULL, first, NULL);
    pthread_create(thrid + 1, NULL, second, NULL);

    pthread_join(thrid[0], NULL);
    pthread_join(thrid[1], NULL);

    return 0;
}
