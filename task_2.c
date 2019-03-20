#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
int main(int argc, char* argv[]){
	execv(argv[1], argv + 1);
	printf("Error with opening the programme");
	return 0;
}
