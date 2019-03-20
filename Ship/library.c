#include "library.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <unistd.h>
#include <sys/wait.h>

int BOAT_SIZE;
int K;
int Z;

#define SEM_BOAT 1
#define SEM_TRAP 2
#define SEM_IN 3
#define SEM_SATISFACTION 4
#define SEM_TOUR 5
#define SEM_EXIT 6

#define SET(command, opt_num, opt_op, opt_flg) \
            command.sem_num = opt_num; \
            command.sem_op = opt_op; \
            command.sem_flg = opt_flg;

#define SETSEM(SEM_NAME, VALUE) \
    if(semctl(semid, SEM_NAME, SETVAL, VALUE) == -1) perror("SETVAL ");
#define V(SEM_NAME) \
    {struct sembuf command = {SEM_NAME, 1, 0}; if (semop(semid, &command,1)==-1){perror("SEM V "); return;}}
#define VK(SEM_NAME, K) \
    {struct sembuf command = {SEM_NAME, K, 0}; if(semop(semid, &command, 1)==-1){perror("SEM VK "); return;}}
#define P(SEM_NAME) \
    {struct sembuf command = {SEM_NAME, -1, 0}; if(semop(semid, &command, 1)==-1){perror("SEM P "); return;}}
#define PK(SEM_NAME, K) \
    {struct sembuf command = {SEM_NAME, -K, 0}; if(semop(semid, &command, 1)==-1){perror("SEM PK "); return;}}
#define WAIT(SEM_NAME) \
    {struct sembuf command = {SEM_NAME, 0, 0}; if(semop(semid, &command, 1)==-1){perror("SEM WAIT "); return;}}



void pass(int semid, int pass_id) {
    //===========
    //struct sembuf command = {};
    //===========

    if (semop(semid, &command, 1) == EXIT_SUCCESS) return;

    P(SEM_TRAP);
    printf("[%d] Walking onto the boat\n", pass_id);
    V(SEM_TRAP);
    V(SEM_IN);

    P(SEM_TOUR);
    printf("[%d]I'm done with trip\n", pass_id);
    V(SEM_SATISFACTION);

    P(SEM_TOUR);

    P(SEM_TRAP);
    printf("[%d] Walk from the boat \n", pass_id);
    V(SEM_TRAP);
}

void boat(int semid) {
    struct sembuf command;
    printf("The boat has approached the shore\n");
    for (int j = 0; j < Z; j++) {
        printf("The boat has put down a trap, taking on people\n");
        VK(SEM_BOAT, BOAT_SIZE);
        VK(SEM_TRAP, K);

        PK(SEM_EXIT, BOAT_SIZE);
        PK(SEM_TRAP, BOAT_SIZE);
        printf("Trap is up, going on a trip\n");
        VK(SEM_TOUR, BOAT_SIZE);

        PK(SEM_SATISFACTION, BOAT_SIZE);

    }
}


int main(int argc, char*argv[]) {
    if(argc < 2) return 0;
    int numPeople = atoi(argv[1]);
    int numTrap = atoi(argv[2]);
    int numShip = atoi(argv[3]);
    int numTrips = atoi(argv[4]);


    return 0;
}