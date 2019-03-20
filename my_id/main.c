#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>

int main() {
    uid_t id = getuid();
    uid_t gid = getgid();
    int list[1024];
    int group = getgroups(1024, list);
    printf("uid = %lld, gid = %lld\n", (long long)id,(long long)gid);
    for(int i = 0; i< group; i++){
        printf("groups:%lld\n",(long long)list[i]);
    }
    return 0;
}