#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <errno.h>
#include <error.h>
#include <string.h>
#include <vector>

#include "car.h"
#include "parseInfo.h"


#define MSGSZ 128
using namespace std;

void turn_right(int id)
{
    cout << "turn_right" << endl;
    return;
}
void turn_left(int id)
{
    cout << "turn_left" << endl;
    return;
}
void go_forward(int id)
{
    cout << "go_forward" << endl;
    return;
}
void stop(int id){
    cout << "stop" << endl;
    return;
}
void finish()
{
    cout << "finish" << endl;
    exit(0);
}

void func(char* com, int id)
{
    cout << com << endl;
    if (!strcmp(com, "turn_right"))
        turn_right(id);
    if (!strcmp(com, "turn_left"))
        turn_left(id);
    if (!strcmp(com, "go_forward"))
        go_forward(id);
    if (!strcmp(com, "stop"))
        stop(id);
    if (!strcmp(com, "end"))
        crash();
    return;
}

void car(int id)
{
    cout << "Car " << id << " begins work" << endl;
    int key = id;
    char null[MSGSZ] = {};
    int msgid = msgget(key, IPC_CREAT | 0666);
    struct msgbuf buf;
    buf.mtype = 1;
    strcpy(buf.mtext, null);
    while (errno == 0) {
        errno = 0;
        msgrcv(msgid, &buf, MSGSZ, 1, 0);
        cout << id << ": Get: " << buf.mtext << endl;
        func(buf.mtext, id);
        strcpy(buf.mtext, null);
    }
    cout << "Car crushed" << endl;
    exit(0);
}

int send_command(int id, char* com, int n, int* msid)
{
    struct msgbuf msg;
    if (id == 0) {
        for (int i = 0; i <= n; ++i)
            msgctl(msid[i+1], IPC_RMID, NULL);
        func(com, id);
    }
    msg.mtype = 1;
    strcpy(msg.mtext, com);
    int msgqid = msgget(id, IPC_CREAT | 0666);
    msgsnd(msgqid, &msg, strlen(msg.mtext)+1, IPC_NOWAIT);
    cout << "Success" << endl;
    return 0;
}

void cars_create(int numb, int* cars)
{
    for (int i = 0; i < numb; ++i)
    {
        cars[i] = i+1;
    }
}

void msg_init(int* id, int* cars, int n)
{
    for (int i = 1; i <= n; ++i) {
        id[i] = msgget(cars[i], IPC_CREAT | 0666);
        msgctl(id[i], IPC_RMID, NULL);
    }
}

void cars_init(int num, int* cars)
{
    int pid;
    for (int i = 0; i < num; ++i) {
            pid = fork();
            if (pid == 0)
                car(cars[i]);
    }
}

int main()
{
    cout << "How many cars?" << endl;
    int    n;
    cin >> n;
    int cars[n];

    cars_create(n, cars);

    int msid[n+1]       = {};
    msg_init(msid, cars, n);
    cars_init(n, cars);

    char command[100]   = {};
    int id              = 0;
    errno               = 0;

    while (errno == 0) {
        cin >> id >> command;
        send_command(id, command, n, msid);
    }

    for (int i = 0; i <= n; ++i)
        msgctl(msid[i+1], IPC_RMID, NULL);

    cout << "Server crashed" << endl;
    wait(NULL);

    return 0;
}