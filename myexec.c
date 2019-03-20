#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
int main(int argc, char* argv[]){
	int num = 0;
    struct timeval tv1, tv2;
	gettimeofday(&tv1, NULL);
	//fork();
    int pin = fork();
    if(pin == 0)
	    execv(argv[1], &argv[1]);
	gettimeofday(&tv2, NULL);
	printf("time = %d\n", (tv2.tv_sec-tv1.tv_sec)*1000000+(tv2.tv_usec-tv1.tv_usec));
    int wait_p;
    pid_t ret = 0;
    ret = wait(&wait_p);
    return 0;
}
