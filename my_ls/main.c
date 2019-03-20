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
    bool all, list, numeric, Recursive, inode, directory;
    int num, not_show_hidden;
};
//NEW FUNCTIONS: LSTAT, READDIR, OPENDIR
//GETOPT, GETPWUID, getgrgid, closedir, getpwnam

void init_opts(struct options *o_p){
    memset(o_p, 0, sizeof (*o_p));
}

void parsing(int argc, char* argv[], struct options* options) {
    int opt = 0;
    init_opts(options);

    //do while no required argument is found in the
    //input string

    while ((opt = getopt(argc, argv, optString)) != -1) {
        options->num++;
        switch (opt) {
            case ('a'):
                options->all = 1;
                break;
            case ('l'):
                options->list = 1;
                options->not_show_hidden++;
                break;
            case ('n'):
                options->numeric = 1;
                options->not_show_hidden++;
                break;
            case ('R'):
                options->Recursive = 1;
                options->not_show_hidden++;
                break;
            case ('i'):
                options->inode = 1;
                options->not_show_hidden++;
                break;
            case ('d'):
                options->directory = 1;
                break;
            case ('?'):
                printf("\nThis key isn't defined\n");
                break;
            default:
                printf("\nThe return value is different\n");
                break;
        }
    }
}

void print_filetype(mode_t mode)
{
    switch (mode & S_IFMT) {
        case S_IFBLK:  printf("block device ");            break;
        case S_IFCHR:  printf("character device ");        break;
        case S_IFDIR:  printf("directory ");               break;
        case S_IFIFO:  printf("FIFO/pipe ");               break;
        case S_IFLNK:  printf("symlink ");                 break;
        case S_IFREG:  printf("regular file ");            break;
        case S_IFSOCK: printf("socket ");                  break;
        default:       printf("unknown? ");                break;
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
    if(dfd == NULL){
        fprintf(stderr, dir);
        perror("cannot open dir in is_in_dir: ");
        return 0;
    }
    /*
    int sc = 0;
    scanf("%d", &sc);
     */
    /*
    struct dirent* entry = (struct dirent*)calloc(sizeof(struct dirent), 1);
    struct dirent* result;

    int res = readdir_r(dfd, entry, &result);

    while (res == 0)
    {
        if (strcmp(pathname, result->d_name) == 0) {
            closedir(dfd);
            return 1;
        }
        res = readdir_r(dfd, entry, &result);
    }
     */
    struct dirent* dirent = readdir(dfd);
    while (dirent)
    {
        if (strcmp(pathname, dirent->d_name) == 0) {
            closedir(dfd);
            return 1;
        }
        dirent = readdir(dfd);
    }

    //free(entry);
    fprintf(stderr, "file \'%s\' not found\n", pathname);

    closedir(dfd);
    return 0;
}

void file_information(char* dir, char* pathname, struct options options){

    if (!is_in_dir(dir, pathname))
        return;

    int secret_dir=pathname[0]=='.';
    int no_options = options.num==0;

    //if((options.list||options.numeric||options.inode||options.Recursive)&&secret_dir)
    if((options.not_show_hidden>0)&&secret_dir)
        return;
    if(no_options&&secret_dir)
        return;

    //default structure, defined by stat function
    struct stat buf;
    struct group *grp;
    struct passwd *pwd;

    char* name = pathname;

    //if(dir!="."){
    //printf("len dir=%d----%d\n",strlen(dir), strlen(pathname));
    char*new_string = (char*)calloc(strlen(dir)+strlen(pathname)+2, sizeof(char));
    strcat(new_string,dir);
    strcat(new_string,"/");
    strcat(new_string,pathname);
    pathname = new_string;
    //}

    current_dir = dir;

    //getting information on file, contained in struct buf
    //printf("pathname = %s\n",pathname);
    int result_stat = lstat(pathname, &buf);
    if(result_stat==-1){
        fprintf(stderr, "Problem with stat ");
        perror(pathname);
        return;
    }

    //printf("\n******************************\n");
    if(options.numeric||options.list) {
        printf(" %s\b", ctime(&buf.st_ctime));
        printf(" %lld ", (long long) buf.st_size);
        print_permissions((unsigned long) buf.st_mode);
        printf(" %d ", (int)buf.st_nlink);
    }
    if(options.list){
        //buf.st_gid is id of group and getgrgid therefore
        //can tell the name of group
        grp = getgrgid(buf.st_gid);
        printf(" %s ", grp->gr_name);

        //print the name of the owner
        pwd = getpwuid(buf.st_uid);
        if(pwd!=NULL)
            printf(" %s ", pwd->pw_name);
        else
            printf("%d ", buf.st_uid);

    }
    print_filetype(buf.st_mode);

    if((((buf.st_mode)&S_IFMT)==S_IFLNK)&&options.list){
        char link_name[1024]={0};
        readlink(pathname,link_name, sizeof(link_name));
        printf("%s->%s ",name,link_name);
    }
    else
        printf(" %s ", name);

    if(options.numeric)
        printf(" %ld %ld ", (long) buf.st_uid, (long) buf.st_gid);
    if(options.inode)
        printf(" %d ", (int)buf.st_ino);
    printf("\n");
    free(new_string);
    printf("\n");
    //*is_dir = (buf.st_mode & S_IFMT & S_IFDIR)!=0 ? 1 : 0;
}


//shows the contents of the given dir
//according to the arguments of "ls"
void show_dir_files(char* dir,char *filename, struct options opts) {
    DIR *directory = opendir(filename);
    if(directory == NULL){
        fprintf(stderr, filename);
        perror("cannot open dir in show_dir_files: ");
        return;
    }
    struct dirent *dirent = readdir(directory);
    //CREATE A CHECK IF THE FILE IS IN DIRECTORY
    /*if (!is_in_dir(dir, filename))
        return;
    */
    //array of directories names
    int calloc_size = 30000;
    char **dir_arr = (char **) calloc((size_t) calloc_size, sizeof(char *));
    //show hidden directories option
    int show_hidden_dirs = opts.all;

    //indicates to show "." file if -d is sent as argument
    long int current_dir_index = 0;

    int link_current_dir=0;

    long int cur_dir = 0;
    while (dirent) {
        //the directory is HIDDEN
        int hidden_dir = (dirent->d_name[0] == '.');

        //link to the current directory contains only "."
        link_current_dir=strcmp(filename, dir)==0;
        //link_current_dir = (dirent->d_name[0] == '.') && strlen(dirent->d_name)==1;
        printf("%lo %c %d\n",cur_dir, dirent->d_name[0],(int)strlen(dirent->d_name));

            //do not omit the hidden directories
        if ((!(!opts.all && hidden_dir))||link_current_dir) {
            if (cur_dir >= calloc_size) {
                calloc_size *= 2;
                char** p = realloc(dir_arr, calloc_size * sizeof(char *));
                assert_polite(p!=NULL, "Dir array realloc\n");
                dir_arr = p;
            }
            //array of names of directories
            dir_arr[cur_dir] = dirent->d_name;
        }

        //if using -d key
        if (opts.directory && link_current_dir)
            current_dir_index = cur_dir;

        cur_dir++;
        dirent = readdir(directory);
    }

    current_dir = filename;
    for (int i = 0; i < cur_dir; ++i) {
        printf("dir_arr[%d] = %s\n", i, dir_arr[i]);
    }
    if(link_current_dir&&opts.directory) {
        printf(" %s \n", filename);
        return;
    }

    else {
        for (long int i = 0; i < cur_dir; ++i){
            if(dir_arr[i]!=NULL)
                file_information(filename, dir_arr[i], opts);
        }
    }
    /*for(long int i =0; i<cur_dir;++i)
        printf("%d-",is_dir_array[i]);*/
    int closeDirResult = closedir(directory);
    assert_polite(closeDirResult!=-1, "Didn't close the directory\n");
    free(dir_arr);

}

int is_recursive_dir(char *prev, char *next){
    //if current directory is not a link to parent or itself
    if (!strcmp(".", next) || !strcmp("..", next))
        return 0;

    char path[2048];
    sprintf(path, "%s/%s", prev, next);
    struct stat sb;

    int res_lstat = lstat(path, &sb);
    if(res_lstat<0){
        fprintf(stderr, path);
        perror("Error with lstat ");
        return 0;
    }

    return S_ISDIR(sb.st_mode);
}

void show_each_dir_recursively(char* dir, struct options opts){
    DIR* directory = opendir(dir);
    if(directory==NULL) {
        fprintf(stderr, "cannot open directory in recursively: ");
        perror(dir);
        return;
    }

    struct dirent* dirent = readdir(directory);

    printf("\n%s:\n", dir);
    while (dirent)
    {
        int omit_hidden = !opts.all && dirent->d_name[0] == '.';

        if (!omit_hidden)
        {
            //if (opts.list||opts.inode||opts.numeric||opts.all)
            if (opts.not_show_hidden>0 && !opts.Recursive)
                show_dir_files(dir, dirent->d_name, opts);
            else if(opts.not_show_hidden>0)
                file_information(dir, dirent->d_name,opts);
            else
                printf("%s\n", dirent->d_name);
        }

        if (is_recursive_dir(dir, dirent->d_name)) {
            char* next = (char*)calloc(strlen(dir)+strlen(dirent->d_name)+2,sizeof(char));
            //char next[1024];
            sprintf(next, "%s/%s", dir, dirent->d_name);

            //printf("******Next directory is %s\n", next);
            show_each_dir_recursively(next, opts);
            free(next);
        }
        dirent = readdir(directory);
    }

    closedir(directory);
}


void run_ls(int count, char* argv[], struct options options) {
    //optind is a number of the next argument created by getopt()
    //numOfFilesToOpen===files in the command line to apply ls
    int numOfFilesToOpen=count - optind;
    int zeroFiles = numOfFilesToOpen==0;
    if (options.directory)
    {
        if(zeroFiles) {
            show_dir_files(".", ".", options);
            return;
        }
        for(int i =optind; i<count;i++)
            show_dir_files(argv[i], argv[i],options);
        return;
    }
    else if(zeroFiles) {
        //if ((options.list || options.numeric || options.inode || options.all) && !options.Recursive) {
        if (options.not_show_hidden>0 && !options.Recursive) {
            show_dir_files(".", ".", options);
        }

        else if (options.Recursive) {
            show_each_dir_recursively(".", options);
        } else {
            show_dir_files(".", ".", options);
        }
    }
    else{
        for (int i = optind; i < count; ++i)
        {
            // display directory name
            // for multiple directories
            if (numOfFilesToOpen>1)
                printf("\n%s:\n", argv[i]);
            if(!options.Recursive&&!options.all)
                show_dir_files(".",argv[i], options);
            else if(!options.all)
                show_each_dir_recursively(argv[i],options);
            else{
                show_dir_files(".", argv[i],options);
            }
        }
    }
 }


int main(int argc, char* argv[]) {
    struct options o_p;
    parsing(argc, argv, &o_p);

    run_ls(argc, argv, o_p);
    return 0;
}