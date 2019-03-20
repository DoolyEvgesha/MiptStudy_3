#include <stdio.h>

int main() {
    char a[]="bye";
    printf("%d\n", sizeof(a));
    char *s="bye";
    printf("%d\n",sizeof(s));
    char k[5];
    printf("%d\n",sizeof(k));

    return 0;
}