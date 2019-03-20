//==================================
//Runners, using msgget, msgctl, msgrcv, msgsnd,
//==================================

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>

#define MSGSZ 128

#define READY 1
#define START 3
#define FINISH 2

#define my_assert(ERR_MSG, CONDITION) \
do{ \
    if(!(CONDITION)){ \
        fprintf(stderr, ERR_MSG": %s\n", strerror(errno)); \
        exit(errno); \
    } \
}while(0)

int msgid;
long numRun;

struct message{
    long mtype;           // Тип сообщения
    char mtext[MSGSZ];    // Само сообщение, длиной MSGSZ.
};

static void send_msg(int msgtype, int number) {
    struct message msg;
    msg.mtype = msgtype;
    int res = snprintf(msg.mtext, sizeof(msg.mtext), "%d", number);
    my_assert("SNPRINTF", res >= 0);

    res = msgsnd(msgid, (void *) &msg, sizeof(msg.mtext), 0);
    //res = msgsnd(msgid, (void *) &msg, sizeof(msg.mtext), IPC_NOWAIT);
    //printf("%s\n", msg.mtext);
    my_assert("MSGSND", res >= 0);
}

static void get_msg(int msgtype) {
    struct message msg;

    ssize_t res = msgrcv(msgid, (void *) &msg, sizeof(msg.mtext), msgtype, 0);
    my_assert("MSGRCV", res >= 0);
}

long countTime(long before, long after){
    return after - before;
}



void judge(){
    printf("BATYA v zdanii\n");

    for(int i = 0; i < numRun; i++){
        get_msg(READY);
        printf("BATYA says that #%d is ready\n", i);
    }

    clock_t start, end;
    double cpu_time_used;
    start = clock();

    send_msg(START, 0); //0 is default number for the judge
    get_msg(FINISH);

    end = clock();
    printf("TIME IS %f\n", ((double) (end - start))*100000 / CLOCKS_PER_SEC);

    exit(EXIT_SUCCESS);
}

void runner(int index){
    printf("Oyu sobaki, ya Naruto Uzumaki #%d\n", index);

    send_msg(READY, index);
    printf("Naruto #%d is ready\n", index);


    usleep(1000);
    get_msg(START + index);
    printf("Naruto #%d started to run\n", index);
    //usleep(1000);
    printf("Naruto #%d finished\n", index);

    if(index != numRun - 1)
        send_msg(START + index + 1,index);//if not the last runner
    else
        send_msg(FINISH, index);//the last one sends a message about finishing the race

    exit(EXIT_SUCCESS);
}

int main(int argc, char* argv[]) {
    if(argc != 2) exit(EXIT_FAILURE);
    numRun = strtol(argv[1], 0, 10);

    msgid = msgget(IPC_PRIVATE, 0700);
    my_assert("msgget", msgid != -1);

    pid_t pid = fork();
    if (pid == 0)
        judge();

    for (int i = 0; i < numRun; i++) {
        pid = fork();
        if (pid == 0)
            runner(i);
    }

    for (int i = 0; i < numRun + 1; i++)
        wait(NULL);

    int res = msgctl(msgid, IPC_RMID, NULL);
    my_assert("msgctl", res >= 0);//IPC_RMID -- Операция удаления очереди сообщений с дескриптором msqid

    return 0;
}