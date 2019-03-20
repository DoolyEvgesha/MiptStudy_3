#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>

#define WORD_SIZE 20
#define STRING_SIZE 100
#define MAX_WORDS 50
#define ARGS_NUMBER 6
#define size_buf 1024
#define MAX_CMDS 25
int num;
struct sep_com {
    char args[ARGS_NUMBER][WORD_SIZE];
};

void add_item(struct sep_com *p, struct sep_com a, int *num_items) {
    if (*num_items < MAX_CMDS) {
        p[*num_items] = a;
        *num_items += 1;
    }
}

int main(int argc, char *argv[]) {
    while (1) {
        char string[STRING_SIZE] = {0};
        int command_number = 0;
        char words[MAX_WORDS][WORD_SIZE];
        for (int j = 0; j < MAX_WORDS; j++)
            memset(words[j], 0, WORD_SIZE);
        printf("$-->");
        //-------------------------------------
        //parsing string
        //-------------------------------------
        if (strcmp(fgets(string, STRING_SIZE, stdin), "exit\n") == 0)
            break;
        int i = 0;
        int res = sscanf(string, "%s", words[i]);
        size_t add = 0;
        while (res != EOF) {
            printf("Input: %s\n", words[i]);
            size_t size = strlen(words[i]) + 1;
            add += size;
            i++;
            res = sscanf(string + add, "%s", words[i]);
            command_number++;
        }
        //======================================
        //creating an array of structures
        //======================================
        struct sep_com cms[MAX_CMDS] = {0};//structure array of commands and arguments
        int curr_word = 0;
        int next_separator = 0;
        int num_items = 0;
        while (words[curr_word][0] != '\0') {
            while ((strcmp(words[next_separator], "|") != 0) && words[next_separator][0] != '\0')
                next_separator++;
            struct sep_com arg = {0};
            for (int f = 0; f < next_separator - curr_word; ++f) {
                strcpy(arg.args[f], words[curr_word + f]);
            }
            curr_word = next_separator + 1;
            next_separator += 1;
            add_item(cms, arg, &num_items);
        }
        //========================================
        //starting fork and pipe
        //========================================
        int fd[2];
        pid_t pid = 0;
        char buf[size_buf];
        if (pipe(fd) < 0) {
            printf("Error: pipe doesn't work\n");
            return 1;
        }
        //------------------------------
        while (num < command_number);
        if (pid < 0) {
            perror("Error with fork\n");
            return 1;
        }
        //==============================
        else if (pid > 0) {// parent
            close(fd[1]);
            execl(words[0], words[1]);
            wait(NULL);
        }//****************************
        else {// child
            close(1);
            dup(fd[1]);
            close(fd[1]);
            close(fd[0]);
            num++;
            // CREATE COMMAND NUMBERS AND WRITE FORKS
            execl(words[num], words[1]);
            return 3;
        }
        //==============================
    }
    return 0;
}
