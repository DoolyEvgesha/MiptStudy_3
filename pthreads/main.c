#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <error.h>
#include <pthread.h>
long int num;
pthread_mutex_t lock;
pthread_t ntid[1000000];

/*void printids(const char *s) {
    pid_t pid;
    pthread_t tid;
    pid = getpid();
    tid = pthread_self();
    printf("%s pid %lu tid %lu (0x%lx)\n", s, (unsigned long)pid, (unsigned long)tid, (unsigned long)tid);
}
*/
/*void *thr_fn(void *arg) {
    printids("new thread: ");
    return((void *)0);
}*/

void *thr_prnt(void *arg){
    int mycount = 0;
    for(int i = 0; i < (long int)arg; i++)
        mycount++;
    /*for(int i =0; i<(long int)arg; i++) {
        pthread_mutex_lock(&lock);
        num++;
        pthread_mutex_unlock(&lock);
    }*/
    pthread_mutex_lock(&lock);
    num+=mycount;
    pthread_mutex_unlock(&lock);
    return NULL;
}

int main(int argc, char* argv[]) {
    int err;
    char* endptr = 0;
    long int n = strtol(argv[1],&endptr,10);
    long int k = strtol(argv[2],&endptr,10);
    if (pthread_mutex_init(&lock, NULL) != 0) {
        printf("\n mutex init failed\n");
        return 1;
    }

    for(int i = 0; i < n;i++){
        err = pthread_create(&ntid[i], NULL, thr_prnt, (void*)k);
        if(err!=0){
            perror("Error with pthread create");
            return 1;
        }
        //printids("new thread: ");
    }
    for(int i = 0; i < n;i++)
        pthread_join(ntid[i], NULL);
    //printids("main thread:");
    //sleep(1);
    printf("Global number is %ld\n", num);
    pthread_mutex_destroy(&lock);
    return 0;
}
