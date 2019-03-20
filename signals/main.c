#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#define BUFFSIZE 128

#define MY_ASSERT(MSG, COND) \
    do{                      \
    if(!COND){               \
        perror(MSG);         \
        exit(errno);         \
        }                    \
    }while(0)

#define MY_SIG(ACT, FUNC, MASK) \
{ACT.sa_handler = FUNC; sigemptyset (&ACT.sa_mask); sigaction (MASK, &ACT, NULL);}

#define KILL(pid, SIG) \
        {if(kill(pid, SIG) == -1){perror("kill"); exit(errno);}}
        /*kill just sends a signal, nothing more*/

#define ALARM                  \
    do{                        \
        alarm(1);              \
        sigsuspend(&signMask); \
        alarm(0);              \
    }while(0) /*after 1 second sends a signal to kill the process*/
/*Повторный вызов функции alarm () приводит к отмене действия предыдущего вызова
 * (если сигнал SIGALRM еще не был сгенерирован), и установке нового времени генерации сигнала
 * Если аргумент seconds равен нулю, то функции alarm() только отменяет предыдущий вызов этой функции*/

#define WRITE					\
	do {					    \
		int bytes = 0;			\
		int shift = 0;			\
		do {				    \
			bytes = write (STDOUT_FILENO, buffer + shift, myIndex);\
			shift += bytes;		\
			myIndex -= bytes;	\
		} while (myIndex);		\
	} while (0)


char buffer[BUFFSIZE];
char bit = 0;
sigset_t signMask;
int myIndex = 0;

void u1Handler() {
    bit = 1;
}

void u2Handler() {
    bit = 0;
}

void childHandler() {
    if (!bit) {
        WRITE;
        exit (EXIT_SUCCESS);
    } else {
        exit(errno);
    }
}


void child(pid_t pid, const char* file){
    int fd = open (file, O_RDONLY);
    MY_ASSERT("read", fd != -1);

    int ret_read = 0;
    do {
        ret_read = read (fd, buffer, BUFFSIZE);

        char byte = 0, mask = 128; // 128 == 10 000 000

        for (int i = 0; i < ret_read; i++) {
            KILL(pid, SIGUSR1);
            ALARM; //alarm sets time in seconds, before SIGALRM goes off

            byte = buffer[i];

            while(mask) //mask from 1 to 10 000 000
            {
                if(mask & byte){
                    KILL(pid, SIGUSR1); //send a signal to parent about a certain char
                } else {
                    KILL(pid, SIGUSR2); //there is no char
                }//SIGUSR1 and SIGUSR2 send signals 1 or 0 to the other process to receive the information

                mask = (mask >> 1) & 127; //127 == 1 111 111

                ALARM;// only signals set in Set can wake up the program
                      //sigsuspend lets the waiting processes to act
            }

            mask = 128;
        }
    } while (ret_read);

    close (fd);

    KILL(pid, SIGUSR2);
    exit (EXIT_SUCCESS);
}

void parent(pid_t pid){

    int bitnum = 0;
    char symbol = 0;

    do {
        ALARM;

        if (bitnum == 0) {
            KILL(pid, SIGUSR1); //send a signal about receiving the information
            ALARM;
        }

        bitnum++;
        symbol = (symbol << 1) + bit;
        if (bitnum == 8) { // 2^8 = 256
            buffer[myIndex++] = symbol;
            bitnum = 0;
            if (myIndex == BUFFSIZE)
                WRITE;
        }

        KILL(pid, SIGUSR1);

    } while (1);

}

int main(int argc, char* argv[]) {
    struct sigaction u1_act = {0}; //set certain action to the signal
    MY_SIG(u1_act, u1Handler, SIGUSR1); //if it receives SIGUSR1 signal => do u1_hldr

    struct sigaction u2_act = {0};
    MY_SIG(u2_act, u2Handler, SIGUSR2);

    struct sigaction ch_act = {0};
    MY_SIG(ch_act, childHandler, SIGCHLD);

    struct sigaction al_act = {0};
    MY_SIG(al_act, exit, SIGALRM);

    sigfillset (&signMask); //memset the signal mask
    sigdelset (&signMask, SIGINT); //delete the SIGINT signal from our set
    sigdelset (&signMask, SIGCHLD);
    sigdelset (&signMask, SIGALRM);

    sigprocmask (SIG_SETMASK, &signMask, NULL);
    //if we choose SIG_SETMASK, then
    // the set of blocked signals is set to the argument set

    sigdelset (&signMask, SIGUSR1);
    sigdelset (&signMask, SIGUSR2);

    pid_t ppid = getpid(); //parent id
    pid_t pid = fork (); // child id

    if (pid == -1) {
        perror ("fork");
        exit(-1);
    }

    if (pid == 0) {
        child (ppid, argv[1]);
    } else {
        parent (pid);
    }

    return 0;

}