#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <error.h>
#include <pthread.h>
int num;
pthread_t ntid;
void printids(const char *s) {
    pid_t pid;
    pthread_t tid;
    pid = getpid();
    tid = pthread_self();
    printf("%s pid %lu tid %lu (0x%lx)\n", s, (unsigned long)pid, (unsigned long)tid, (unsigned long)tid);
}
void *thr_fn(void *arg) {
    printids("new thread: ");
    return((void *)0);
}
void *thr_prnt(void *arg){
    num += (long int)arg;
    return((void*)0);
}

int main(int argc, char* argv[]) {
    int err;
    char* endptr = 0;
    long int n = strtol(argv[1],&endptr,10);
    long int k = strtol(argv[2],&endptr,10);
    printf("n = %d\n", n);
    for(int i = 0; i < n;i++){
        err = pthread_create(&ntid, NULL, thr_prnt, (void*)k);
        printids("new thread: ");
    }
    printids("main thread:");
    sleep(1);
    printf("Global number is %d\n", num);
    return 0;
}
