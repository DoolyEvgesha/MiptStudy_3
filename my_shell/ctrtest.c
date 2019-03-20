#include <stdio.h>

int main()
{
	char str[100] = {};
	fgets(str, 100, stdin);
	printf("%d\n", str[0]);
	return 0;
}

