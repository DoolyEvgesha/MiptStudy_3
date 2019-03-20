#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <sysexits.h>
#include <stdlib.h>
//struct passwd {
    //char   *pw_name;       /* username */
    //char   *pw_passwd;     /* user password */
    //uid_t   pw_uid;        /* user ID */
    //gid_t   pw_gid;        /* group ID */
  //  char   *pw_gecos;      /* user information */
    //char   *pw_dir;        /* home directory */
  //  char   *pw_shell;      /* shell program */
//};

//getpwuid, getpwnam, getgrnam, getgrgid
int main(int argc, char* argv[]) {
    struct group *grp;
    struct passwd *pwd;
    pwd = getpwnam(argv[1]);
    printf("Name: %s; UID: %ld; GID: %ld\n", pwd->pw_name, (long) pwd->pw_uid, (long)pwd->pw_gid);
//    grp = getgrgid();
//    printf("group: %s\n", grp->gr_name);

    pwd = getpwuid(pwd->pw_uid);
    printf("username: %s\n", pwd->pw_name);
    return 0;
}
