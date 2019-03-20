#include <stdio.h>
#include <time.h>

int main(){
	time_t t = time(NULL);
	printf("Bla\n");
	printf("wowfdjslfjldsjfldjsl\n");
	time_t t1 = time(NULL);
	printf("Time now is %d\n", t-t1);
	return 0;
}
