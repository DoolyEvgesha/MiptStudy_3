#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
/*
struct stat {
    dev_t     st_dev;         /* ID of device containing file */
    ino_t     st_ino;         /* inode number */
    mode_t    st_mode;        /* file type and mode */
    nlink_t   st_nlink;       /* number of hard links */
    uid_t     st_uid;         /* user ID of owner */
    gid_t     st_gid;         /* group ID of owner */
    dev_t     st_rdev;        /* device ID (if special file) */
    off_t     st_size;        /* total size, in bytes */
    blksize_t st_blksize;     /* blocksize for filesystem I/O */
    blkcnt_t  st_blocks;      /* number of 512B blocks allocated */
/*
    struct timespec st_atim;  /* time of last access */
    //struct timespec st_mtim;  /* time of last modification */
  //  struct timespec st_ctim;  /* time of last status change */
//};

char* get_char(int number){
    char *word = 0;
    switch(number) {
        case 0:
            word = "---";
            break;
        case 1:
            word = "--x";
            break;
        case 2:
            word = "-w-";
            break;
        case 3:
            word = "-wx";
            break;
        case 4:
            word = "r--";
            break;
        case 5:
            word = "r-x";
            break;
        case 6:
            word = "rwx";
            break;
        default:
            printf("Error occured\n");
    }
    return word;
}

int main(int argc, char* argv[]) {
    struct stat buf;
    if (stat(argv[1], &buf) == -1) {
        perror("stat");
        return 1;
    }
    printf("Ownership: UID=%ld   GID=%ld\n", (long) buf.st_uid, (long) buf.st_gid);
    printf("Block size: %ld bytes\n",(long) buf.st_blksize);
    printf("File size: %lld bytes\n", (long long) buf.st_size);
    printf("Blocks: %lld\n", (long long) buf.st_blocks);
    printf("Last status change: %s", ctime(&buf.st_ctime));
    printf("Last file access: %s", ctime(&buf.st_atime));
    printf("Last file modification: %s", ctime(&buf.st_mtime));
    printf("Number of hard links: %d\n", (int)buf.st_nlink);
    int mode = (int)buf.st_mode&0777;
    printf("Access: %2o\n", mode);
    int u = ((int)buf.st_mode&0700)>>6;
    int g = ((int)buf.st_mode&070)>>3;
    int o = (int)buf.st_mode&07;
    printf("%o %o %o\n", u, g, o);
    printf("Access: %s-%s-%s\n",get_char(u), get_char(g), get_char(o));
    return 0;
}
