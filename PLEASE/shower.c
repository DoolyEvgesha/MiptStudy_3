#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <errno.h>
#include <stdlib.h>

#define ANYONE 1
#define MAN 2
#define LOCK_M 3
#define WOMAN 4
#define LOCK_W 5
#define CUB 6

typedef struct {
    long mtype;
} msg_t;

int q_sync  = 0;
int q_cub   = 0;
int q_lock  = 0;

void human(int _gender);

int main(int argc, char *argv[]) {
    setbuf(stderr, NULL);
    setbuf(stdout, NULL);

    if(argc != 4) {
        exit(EXIT_FAILURE);
    }

    int N = strtol(argv[1], NULL, 0);
    int M = strtol(argv[2], NULL, 0);
    int W = strtol(argv[3], NULL, 0);

    q_sync = msgget(IPC_PRIVATE, IPC_CREAT | 0600);
    q_cub  = msgget(IPC_PRIVATE, IPC_CREAT | 0600);
    q_lock = msgget(IPC_PRIVATE, IPC_CREAT | 0600);

    msg_t msg = { .mtype = CUB };

    for(int i = 0; i < N; ++i)
        msgsnd(q_cub, &msg, 0, 0);

    msg.mtype = LOCK_M;
    msgsnd(q_lock, &msg, 0, 0);

    msg.mtype = LOCK_W;
    msgsnd(q_lock, &msg, 0, 0);

    for(int i = 0; i < M + W; ++i) {
        if (fork() == 0) {
            human(i < M ? MAN : WOMAN);
            break;
        }
    }

    msg.mtype = ANYONE;
    msgsnd(q_sync, &msg, 0, 0);

    for(int i = 0; i < M + W; ++i)
        wait(NULL);

    msgctl(q_sync, IPC_RMID, NULL);
    msgctl(q_cub,  IPC_RMID, NULL);
    msgctl(q_lock, IPC_RMID, NULL);

    return 0;
}

void enter(int _gender) {
    //here we should create a free pass for one kind of person
    // and exclude the other kind
    //then take a lock from the lock queue
    int lock   = (_gender == MAN) ? LOCK_M : LOCK_W;
    int except = (_gender == MAN) ? WOMAN  : MAN;

    msg_t msg = {};

    msgrcv(q_lock, &msg, 0, lock, 0);
    msgrcv(q_sync, &msg, 0, except, MSG_EXCEPT);

    if(msg.mtype == _gender)
        msgsnd(q_sync, &msg, 0, 0);

    msg.mtype = _gender;
    msgsnd(q_sync, &msg, 0, 0);

    msg.mtype = lock;
    msgsnd(q_lock, &msg, 0, 0);
}

void leave(int _gender) {
    int lock = (_gender == MAN) ? LOCK_M : LOCK_W;

    msg_t msg = {};
    msgrcv(q_lock, &msg, 0, lock, 0);

    msgrcv(q_sync, &msg, 0, _gender, 0);

    if(msgrcv(q_sync, &msg, 0, _gender, IPC_NOWAIT) == -1 && errno == ENOMSG)
        msg.mtype = ANYONE;
    else
        msg.mtype = _gender;

    msgsnd(q_sync, &msg, 0, 0);

    msg.mtype = lock;
    msgsnd(q_lock, &msg, 0, 0);
}

void proceed(int _gender) {
    msg_t msg = {};
    msgrcv(q_cub, &msg, 0, CUB, 0);

    pid_t pid = getpid();
    char *gendc = (_gender == MAN) ? " M" : "*****W";

    printf("%s[%d] start\n",  gendc, pid);
    printf("%s[%d] finish\n", gendc, pid);

    msgsnd(q_cub, &msg, 0, 0);
}

void human(int _gender) {
    setbuf(stdout, NULL);
    setbuf(stderr, NULL);

    enter(_gender);
    proceed(_gender);
    leave(_gender);

    exit(EXIT_SUCCESS);
}