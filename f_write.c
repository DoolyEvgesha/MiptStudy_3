#include <stdio.h>

int main(){
	int i =101;
	fwrite(&i, sizeof(int), 1, stdout);
}
