#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdlib.h>
#include <memory.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

#define assert_polite(CONDITION, ERR_MESSAGE)            \
    if(!(CONDITION))                                     \
    {                                                    \
        perror(ERR_MESSAGE);                             \
        exit(errno);                                     \
    }

int pile[10000] = {0};
int i = 0;
unsigned long overall_size;

int file_information(char* dir, char* pathname){
    struct stat buf;
    float file_size = 0;

    char* name = pathname;
    char*new_string = (char*)calloc(strlen(dir)+strlen(pathname)+2, sizeof(char));
    strcat(new_string,dir);
    strcat(new_string,"/");
    strcat(new_string,pathname);
    pathname = new_string;

    int result_stat = lstat(pathname, &buf);
    assert_polite(result_stat != -1, "lstat file_information");

    if(!S_ISREG(buf.st_mode)) return 0;
    if(buf.st_nlink == 1) return buf.st_blocks/2;

    for(int j = 0; j < i; j++){
        if(pile[j] == buf.st_ino){
            free(new_string);
            return file_size;
        }
    }
    pile[i++] = (int)buf.st_ino;

    free(new_string);
    return buf.st_blocks/2;
}

int is_recursive_dir(char *prev, char *next){
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

//unsigned long file_size;

void show_contents(char* dir){
    DIR* directory = opendir(dir);
    if(directory == NULL){
        fprintf(stderr, dir);
        perror("Error with lstat ");
        return;
    }

    struct dirent* dirent = readdir(directory);

    unsigned long file_size = 0;
    //printf("\n--%s:", dir);
    while (dirent)
    {
        if(strcmp(dirent->d_name, "..") != 0 && strcmp(dirent->d_name, ".") != 0) {
            file_size += file_information(dir, dirent->d_name);
        }

        if (is_recursive_dir(dir, dirent->d_name)) {
            char* next = (char*)calloc(strlen(dir)+strlen(dirent->d_name)+2,sizeof(char));
            sprintf(next, "%s/%s", dir, dirent->d_name);
            show_contents(next);
            free(next);
        }
        dirent = readdir(directory);
    }
    printf("%ld %s\n", file_size, dir);
    overall_size += file_size;
    closedir(directory);
}

int main(int argc, char *argv[]) {
    if(argc != 2){
        printf("TOO LITTLE ARGS\n");
        exit(EXIT_FAILURE);
    }
    show_contents(argv[1]);
    printf("OVERALL SIZE IS %ld\n", overall_size);
    return 0;
}