#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/sem.h>
#include <errno.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#define printf(args...)             \
    do {                            \
        printf(args);               \
        fflush(stdout);             \
    } while(0)

#define MSGSZ 1

#define WOMAN 1
#define MAN 2

#define LOCK_W 3
#define LOCK_M 4
#define CAB 5
#define ANYONE 6

int synch;//
int lock; // queues' ids
int cab;  //

long numSpace;
long W;
long M;

int womenInside = 0;
int menInside = 0;
int peopleInside = 0;

typedef struct {
    long mtype;           // Тип сообщения
    char mtext[MSGSZ];    // Само сообщение, длиной MSGSZ.
} message;

void entrance(int sex) {
    //here we should create a free pass for one kind of person
    // and exclude the other kind
    //then take a lock from the lock queue
    message msg = {};
    int key = (sex == WOMAN) ? LOCK_W : LOCK_M; // HOW IS BATYA IN DA SHOWER???
    int excl = (sex == WOMAN) ? MAN : WOMAN; //exclude

    msgrcv(lock, &msg, 0, key, 0);
    msgrcv(synch, &msg, 0, excl, MSG_EXCEPT); //except for the other sex

    if (msg.mtype == sex) {
        msgsnd(synch, &msg, 0, 0);
    }
    msg.mtype = sex;
    msgsnd(synch, &msg, 0, 0);
    msgsnd(lock, &msg, 0, 0);
}

void bathing(int sex, int index) {
    message msg = {};
    msgrcv(cab, &msg, 0, CAB, 0);

    if (sex == WOMAN) {
        printf("W #%d v zdanii\n", index);
        printf("W #%d fini la comedie\n", index);
    } else {
        printf("BATYA #%d v zdanii\n", index);
        printf("BATYA #%d: fini la comedie\n", index);
    }

    msgsnd(cab, &msg, 0, 0);
}

void leave(int sex) {
    message msg = {};
    int key = (sex == WOMAN) ? LOCK_W : LOCK_M;
    msgrcv(lock, &msg, 0, sex, 0);
    if (msgrcv(synch, &msg, 0, sex, IPC_NOWAIT) == -1 && errno == ENOMSG)
        msg.mtype = ANYONE;
    else msg.mtype = sex;

    msgsnd(synch, &msg, 0, 0);
    msg.mtype = key;
    msgsnd(lock, &msg, 0, 0);
}

void human(int sex, int index) {
    setbuf(stdout, NULL);
    setbuf(stderr, NULL);

    entrance(sex);
    bathing(sex, index);
    leave(sex);

    exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[]) {
    setbuf(stdout, NULL);
    if (argc != 4)
        exit(EXIT_FAILURE);

    W = strtol(argv[1], NULL, 10);
    M = strtol(argv[2], NULL, 10);
    numSpace = strtol(argv[3], NULL, 10);
    if (numSpace == 0)
        exit(EXIT_FAILURE);

    synch = msgget(IPC_PRIVATE, IPC_CREAT | 0600);
    cab = msgget(IPC_PRIVATE, IPC_CREAT | 0600);
    lock = msgget(IPC_PRIVATE, IPC_CREAT | 0600);

    message msg;
    msg.mtype = CAB;

    for (int i = 0; i < numSpace; ++i) {
        msgsnd(cab, &msg, 0, 0);
    }

    msg.mtype = LOCK_W;
    msgsnd(lock, &msg, 0, 0);

    msg.mtype = LOCK_M;
    msgsnd(lock, &msg, 0, 0);

    for (int i = 0; i < M + W; ++i) {
        if (fork() == 0) {
            human(i < M ? MAN : WOMAN, i);
            break;
        }
    }

    for (int i = 0; i < W + M; i++)
        wait(NULL);

    msgctl(synch, IPC_RMID, NULL);
    msgctl(cab, IPC_RMID, NULL);
    msgctl(lock, IPC_RMID, NULL);//IPC_RMID -- Операция удаления очереди сообщений с дескриптором msqid

    return 0;
}




