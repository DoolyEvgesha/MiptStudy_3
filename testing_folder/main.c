#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <getopt.h>
#include <grp.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sysexits.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <glob.h>

#define assert_polite(CONDITION, ERR_MESSAGE)            \
    if(!(CONDITION))                                     \
    {                                                    \
        perror(ERR_MESSAGE);                             \
        exit(errno);                                     \
    }

typedef unsigned short bool;
static char* current_dir =".";

static const char *optString = "alnRid";
struct options{
    bool a, l, n, R, i, d;
};
//NEW FUNCTIONS: LSTAT, READDIR, OPENDIR
//GETOPT, GETPWUID, getgrgid, closedir, getpwnam

void init_opts(struct options *o_p){
    o_p->a = 0;
    o_p->l = 0;
    o_p->n = 0;
    o_p->R = 0;
    o_p->i = 0;
    o_p->d = 0;

}

void parsing(int argc, char* argv[], struct options* options) {
    int opt = 0;
    init_opts(options);

    //do while no required argument is found in the
    //input string
    opt = getopt(argc, argv, optString);
    printf("\n opt = %c\n", opt);

    while (opt != -1) {
        switch (opt) {
            case ('a'):
                options->a = 1;
                break;
            case ('l'):
                options->l = 1;
                break;
            case ('n'):
                options->n = 1;
                break;
            case ('R'):
                options->R = 1;
                break;
            case ('i'):
                options->i = 1;
                break;
            case ('d'):
                options->d = 1;
                break;
            case ('?'):
                printf("\nThis key isn't defined\n");
                break;
            default:
                printf("\nThe return value is different\n");
                break;
        }
        opt = getopt(argc, argv, optString);
    }
}

void print_filetype(mode_t mode)
{
    switch (mode & S_IFMT) {
        case S_IFBLK:  printf("block device\n");            break;
        case S_IFCHR:  printf("character device\n");        break;
        case S_IFDIR:  printf("directory\n");               break;
        case S_IFIFO:  printf("FIFO/pipe\n");               break;
        case S_IFLNK:  printf("symlink\n");                 break;
        case S_IFREG:  printf("regular file\n");            break;
        case S_IFSOCK: printf("socket\n");                  break;
        default:       printf("unknown?\n");                break;
    }
}

void print_permissions(unsigned long mode)
{
    putchar((mode & S_IRUSR) ? 'r' : '-');
    putchar((mode & S_IWUSR) ? 'w' : '-');
    putchar((mode & S_IXUSR) ? 'x' : '-');
    putchar((mode & S_IRGRP) ? 'r' : '-');
    putchar((mode & S_IWGRP) ? 'w' : '-');
    putchar((mode & S_IXGRP) ? 'x' : '-');
    putchar((mode & S_IROTH) ? 'r' : '-');
    putchar((mode & S_IWOTH) ? 'w' : '-');
    putchar((mode & S_IXOTH) ? 'x' : '-');
}

int is_in_dir(char* dir, char* pathname){
    DIR* dfd = opendir(dir);

    assert_polite(dfd!=0, "Error: couldn't open directory");
    struct dirent* dirent = readdir(dfd);

    while (dirent)
    {
        if (strcmp(pathname, dirent->d_name) == 0) {
            closedir(dfd);
            return 1;
        }
        dirent = readdir(dfd);
    }

    fprintf(stderr, "file \'%s\' not found\n", pathname);

    closedir(dfd);
    return 0;
}

void file_information(char* dir, char* pathname, struct options options, int* is_dir){

    if (!is_in_dir(dir, pathname))
        return;

    //default structure, defined by stat function
    struct stat buf;
    struct group *grp;
    struct passwd *pwd;

    //getting information on file, contained in struct buf
    if (stat(pathname, &buf) == -1) {
        perror("stat");
        exit(errno);
    }

    current_dir = dir;

    printf("\n******************************\n");
    if(options.n||options.l) {
        printf("Last status change: %s", ctime(&buf.st_ctime));
        printf("File size: %lld bytes\n", (long long) buf.st_size);
        printf("Access:");
        print_permissions((unsigned long) buf.st_mode);
        printf("\n");
        printf("Number of hard links: %d\n", (int)buf.st_nlink);
    }
    if(options.l){
        //buf.st_gid is id of group and getgrgid therefore
        //can tell the name of group
        grp = getgrgid(buf.st_gid);
        printf("Group: %s\n", grp->gr_name);

        //print the name of the owner
        pwd = getpwuid(buf.st_uid);
        printf("Username: %s\n", pwd->pw_name);
    }
    printf("File type: ");
    print_filetype(buf.st_mode);

    printf("File name: %s\n", pathname);

    if(options.n)
        printf("UID=%ld   GID=%ld\n", (long) buf.st_uid, (long) buf.st_gid);
    if(options.i)
        printf("Inode: %d\n", (int)buf.st_ino);

    *is_dir = (buf.st_mode & S_IFMT & S_IFDIR)!=0 ? 1 : 0;
}


//shows the contents of the given dir
//according to the arguments of "ls"
void show_dir_files(char* dir,char *filename, struct options opts) {
    DIR *directory = opendir(filename);
    assert_polite(directory != NULL, "Problem with opendir\n")
    struct dirent *dirent = readdir(directory);

    //CREATE A CHECK IF THE FILE IS IN DIRECTORY
    if (!is_in_dir(dir, filename))
        return;

    //array of directories names
    int calloc_size = 30000;
    char **dir_arr = (char **) calloc((size_t) calloc_size, sizeof(char *));
    //show hidden directories option
    int show_hidden_dirs = opts.a;

    //indicates to show "." file if -d is sent as argument
    long int current_dir_index = 0;

    long int cur_dir = 0;
    while (dirent) {
        //the directory is HIDDEN
        int hidden_dir = (dirent->d_name[0] == '.');

        //link to the current directory contains only "."
        int link_current_dir = (dirent->d_name[0] == '.') && strlen(dirent->d_name)==1;
        printf("%lo %c %d\n",cur_dir, dirent->d_name[0],(int)strlen(dirent->d_name));

            //do not omit the hidden directories
        if ((!(!opts.a && hidden_dir))||link_current_dir) {
            if (cur_dir >= calloc_size) {
                calloc_size *= 2;
                dir_arr = realloc(dir_arr, calloc_size * sizeof(char *));
            }
            //array of names of directories
            dir_arr[cur_dir] = dirent->d_name;
        }

        //if using -d key
        if (opts.d && link_current_dir)
            current_dir_index = cur_dir;

        cur_dir++;
        dirent = readdir(directory);
    }

    current_dir = filename;

    int is_dir_array[cur_dir];

    for (int i = 0; i < cur_dir; ++i) {
        printf("dir_arr[%d] = %s\n", i, dir_arr[i]);
    }
    if (current_dir_index)
        file_information(filename, dir_arr[current_dir_index], opts, &is_dir_array[current_dir_index]);
    else {
        for (long int i = 0; i < cur_dir; ++i){
            if(dir_arr[i]!=NULL)
                file_information(filename, dir_arr[i], opts, &is_dir_array[i]);
        }
    }
    /*for(long int i =0; i<cur_dir;++i)
        printf("%d-",is_dir_array[i]);*/
    int closeDirResult = closedir(directory);
    assert_polite(closeDirResult!=-1, "Didn't close the directory\n");
    free(dir_arr);

}

int recursive_dir(char* prev, char*next){
    //if current directory is not a link to parent or itself
    if (!strcmp(".", next) || !strcmp("..", next))
        return 0;

    char path[2048];
    sprintf(path, "%s/%s", prev, next);
    struct stat sb;

    int res_lstat = lstat(path, &sb);
    assert_polite(res_lstat>=0, "Error with lstat\n");
    return S_ISDIR(sb.st_mode);
}

void show_each_dir_recursively(char* dir, struct options opts){
    DIR* directory = opendir(dir);
    struct dirent* dirent = readdir(directory);

    printf("\n%s:\n", dir);
    while (dirent)
    {
        int omit_hidden = !opts.a && dirent->d_name[0] == '.';

        if (!omit_hidden)
        {
            if (opts.l||opts.i||opts.n||opts.a)
                show_dir_files(dir, dirent->d_name, opts);
            else
                printf("%s\n", dirent->d_name);
        }

        if (recursive_dir(dir, dirent->d_name)) {
            char next[1024];
            sprintf(next, "%s/%s", dir, dirent->d_name);

            //printf("******Next directory is %s\n", next);
            show_each_dir_recursively(next, opts);
        }
        dirent = readdir(directory);
    }

    closedir(directory);
}


void run_ls(int count, char* argv[], struct options options) {
    //optint is a number of the next argument
    if (options.d)
    {
        show_dir_files(".",".", options);
        return;
    }
    if((options.l||options.n||options.i||options.a)&&!options.R){
        show_dir_files(".",".", options);
    }

    if(options.R){
        show_each_dir_recursively(".", options);
    }
 }


int main(int argc, char* argv[]) {
    struct options o_p;
    parsing(argc, argv, &o_p);
    run_ls(argc, argv, o_p);
    return 0;
}