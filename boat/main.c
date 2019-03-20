#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <memory.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>

#define SEM_BOAT 0
#define SEM_TRAP 1
#define SEM_IN 2
#define SEM_EXIT 3

#define my_assert(ERR_MSG, CONDITION)                                        \
if(!(CONDITION))                                                             \
    {                                                                        \
        fprintf(stderr, ERR_MSG": %s\n", strerror(errno));                   \
        exit(errno);                                                         \
    }

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

/* системный вызов semop требует массив таких структур
struct sembuf {   ushort sem_num;   // индекс семафора в массиве
    short  sem_op;    // Выполняемая операция (положительное, отрицательное число или нуль)
    short  sem_flg;   // флаги
}; */
//Первый аргумент semop() есть значение ключа (в нашем случае возвращается semget-ом). Второй аргумент (sops) - это
// указатель на массив операций, выполняемых над семафорами, третий аргумент (nsops) является количеством
// операций в этом массиве.

//если sem_op равен нулю, то вызывающий процесс будет усыплен (sleep()), пока значение семафора не
// станет нулем. Это соответствует ожиданию того, что ресурсы будут использованы на 100%

int semid;

void ship(short numShip, short numTrap, short numTrips){

    VK(SEM_BOAT, numShip);//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // make space in semaphore to certain amount of passengers
    for (int i = 0; i < numTrips; i++) {
        printf("Ship approaches the beach\n");

        V(SEM_IN);//semaphore for the trip

        printf("Ship decends the ladder\n");
        VK(SEM_TRAP,numTrap);
        usleep(1000);

        printf("Ship: close ladder ROUND NUMBER %d\n", i);

        PK(SEM_TRAP, numTrap);

        //allow to leave the ship
        P(SEM_IN); //do not go on a trip during this
        usleep(1000);
    }
    V(SEM_EXIT);
    V(SEM_IN);

    //open ladder
    printf("Ship: open ladder, numTrap = %d\n", numTrap);
    VK(SEM_TRAP, numTrap+1000000);
    //usleep(1000);
    VK(SEM_BOAT, numShip);
    //VK(SEM_TRAP, numTrap);//////F
    printf("Ship leaves the beach\n");

    exit(EXIT_SUCCESS);
}

int check(){
    struct sembuf check = {SEM_EXIT, 0, IPC_NOWAIT};
    int res = semop(semid, &check, 1);
    if (res == -1 && errno == EAGAIN) {
        //printf("Break***********************\n");
        return 1;
    }
    else {
        my_assert("semop", res != -1);
        return 0;
    }
}

void passenger(short passNum){
    while(1)
    {
        printf("Passenger %d wants to go to ship\n", passNum);
        if(check()) break;

        //P(SEM_TRAP);
        P(SEM_BOAT);//===========================
        if(check()) break;

        P(SEM_TRAP);
        if(check()) break;
        //go to ship
        V(SEM_TRAP);
        if(check()) break;

        printf("Passenger %d on ship\n", passNum);

        //WAIT(SEM_IN);
        printf("Passenger %d is tripping\n", passNum);
        if(check()) break;

        printf("Passenger %d leaves the ship\n", passNum);
        //go to ladder
        P(SEM_TRAP);
        //this is where the program stops working!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        //that means that semaphore is "0"

        if(check()) break;

        //free seat on the ship
        V(SEM_BOAT);
        if(check()) break;

        //go to beach
        V(SEM_TRAP);
        if(check()) break;
    }
    V(SEM_BOAT);
    printf("Passenger %d leaves the beach\n", passNum);

    exit(EXIT_SUCCESS);
}

int main(int argc, char*argv[]) {
    if(argc != 5) return 0;
    const short numPeople = (short)strtol(argv[1], NULL, 10);
    const short numTrap =   (short)strtol(argv[2], NULL, 10);
    const short numShip =   (short)strtol(argv[3], NULL, 10);
    const short numTrips =  (short)strtol(argv[4], NULL, 10);

    semid = semget(IPC_PRIVATE, 4, 0700);
    my_assert("semget", semid != -1);

    if (fork() == 0)
        ship(numShip, numTrap, numTrips);

    for (short i = 0; i < numPeople; i++) {
        if (fork() == 0)
            passenger(i);
    }

    for (short i = 0; i <= numPeople; i++)
        wait(NULL);

    int ctl = semctl(semid, 0, IPC_RMID, 0); //delete semaphores from kernel
    my_assert("semctl", ctl != -1);
    printf("It works\n");
    return 0;
}