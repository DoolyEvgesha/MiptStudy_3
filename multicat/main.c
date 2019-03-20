#include <stdio.h>
#include <fcntl.h>           /* For O_* constants */
#include <sys/stat.h>        /* For mode constants */
#include <semaphore.h>

int main() {

    sem_t * sem;

    sem = sem_open();


    return 0;
}