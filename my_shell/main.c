#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <errno.h>
#include <assert.h>

#define WORD_SIZE 20
#define STRING_SIZE 100
#define MAX_WORDS 50
#define ARGS_NUMBER 6
#define size_buf 1024
#define MAX_CMDS 25

struct sep_com {
    char args[ARGS_NUMBER][WORD_SIZE];
};

void add_item(struct sep_com *p, struct sep_com a, int *num_items);
void getting_string(char *string);
void parsing_words(size_t add, int res, int i,char words[][WORD_SIZE], char* string);
void parsing_commands(int* num_items, int curr_word, int next_separator, char* words[], struct sep_com cms[]);
int countPipes(struct sep_com cms[]);
void run_piped_commands(struct sep_com cms[], size_t num_cmds);



int main(int argc, char *argv[]) {
    while (1) {
        char string[STRING_SIZE] = {0};
        char words[MAX_WORDS][WORD_SIZE];
        for (int j = 0; j < MAX_WORDS; j++)
            memset(words[j], 0, WORD_SIZE);
        printf("$-->");
        //-------------------------------------
        //parsing string
        //-------------------------------------
        getting_string(string);
        int i = 0;
        int res = sscanf(string, "%s", words[i]);
        size_t add = 0;
        parsing_words(add, res, i, words, string);
        //======================================
        //creating an array of structures
        //======================================
        struct sep_com cms[MAX_CMDS] = {0};//structure array of commands and arguments
        int curr_word = 0;
        int next_separator = 0;
        int num_cmds = 0;//number of commands
        int j = 0;
        char* str_array[MAX_WORDS]= {NULL};
        while(words[j][0]!=0){
            str_array[j] = words[j];
            j++;
        }
        str_array[j] = words[j];
        parsing_commands(&num_cmds, curr_word, next_separator, str_array, cms);
        //========================================
        //starting fork and pipe
        run_piped_commands(cms, num_cmds);

    }
    return 0;
}
/*
 *
 * ****************************************************************
 *
 */

void add_item(struct sep_com *p, struct sep_com a, int *num_items) {
    if (*num_items < MAX_CMDS) {
        p[*num_items] = a;
        *num_items += 1;
    }
}

void getting_string(char *string){
    char*result = NULL;
    if ((result=fgets(string,STRING_SIZE, stdin)) == NULL)
    {
        printf("exit\n");
        exit(0);
    }
        // strcpy(result, "exit\n");
    //if (strcmp(fgets(string, STRING_SIZE, stdin), "exit\n") == 0)
    if(strcmp(result, "exit\n")==0)
        exit(0);
}

void parsing_words(size_t add, int res, int i,char words[][WORD_SIZE], char* string){
    while (res != EOF) {
        //printf("Input: %s\n", words[i]);
        size_t size = strlen(words[i]) + 1;
        add += size;
        i++;
        res = sscanf(string + add, "%s", words[i]);
    }
}

void parsing_commands(int* num_items, int curr_word, int next_separator, char* words[], struct sep_com cms[]){
    while (words[curr_word] != 0) {
        while ((strcmp(words[next_separator], "|") != 0) && (strcmp(words[next_separator],"")!=0))
            next_separator++;
        struct sep_com arg = {0};

        int f = 0;
        for (; f < next_separator - curr_word; ++f) {
            strcpy(arg.args[f], words[curr_word + f]);
        }
        curr_word = next_separator + 1;
        next_separator += 1;
        add_item(cms, arg, num_items);
    }
}

#define SEND 1
#define RECV 0

#define assert_polite(CONDITION, ERR_MESSAGE)            \
    if(!(CONDITION))                                     \
    {                                                    \
        perror(ERR_MESSAGE);                             \
        exit(errno);                                     \
    }


void run_piped_commands(struct sep_com cms[], size_t num_cmds) {

    int input_pipe[2] = {-1, -1};
    int output_pipe[2] = {-1, -1};

    for(size_t curr_cmd = 0; curr_cmd < num_cmds; ++curr_cmd) {

        // cms[N].args[0] holds the Nth command string, e.g. if the Nth command
        //     is "echo 123 456", the cms[N].args[0] shall be "echoЭ
        char* exe_string = cms[curr_cmd].args[0];
        //printf("\n******current_command = %s******\n", exe_string);

        // cms[N].args[1] through cms[N].args[ARGS_NUMBER] hold the argument
        //     strings for the Nth command. For the command "echo 123 456" those
        //     shall be "123", "456", "", ..., "".
        char *argv[ARGS_NUMBER] = {NULL};
        size_t arg_no = 0;
        for (; arg_no < ARGS_NUMBER && cms[curr_cmd].args[arg_no][0]; ++arg_no)
            argv[arg_no] = cms[curr_cmd].args[arg_no];

        assert(argv[arg_no] == NULL); // Required by execvp!

        //argc == arg_no

        // The output pipe of the previously started command becomes the input
        // pipe of the newly started one.
        if(input_pipe[SEND]!=-1)
            close(input_pipe[SEND]);
        if(input_pipe[RECV]!=-1)
            close(input_pipe[RECV]);

        input_pipe[SEND] = output_pipe[SEND];
        input_pipe[RECV] = output_pipe[RECV];

        // Create a new output (right) pipe unless it's the last command
        if (curr_cmd + 1 != num_cmds) {
            int pipe_result = pipe(output_pipe);
            assert_polite(pipe_result >= 0, "Failed to create a pipe");
        }

        // pid_t my_little_pid = fork();
        pid_t my_pid = fork();
        assert_polite(my_pid >= 0, "Failed to fork");

        if(my_pid == 0) {// CHILD

            // If we are the first command, the input pipe wasn't opened for us.
            if (curr_cmd != 0) {

                // We don't need to write to input pipe, so discard its sending
                // end
                close(input_pipe[SEND]);

                // Redirect the receiving end of the input pipe to stdin
                int dup2_result = dup2(input_pipe[RECV], STDIN_FILENO);

                assert_polite(dup2_result >= 0,
                              "Failed to reassign stdin to the input pipe");
            }

            // If we are the last command, the output pipe wasn't opened for us.
            if (curr_cmd + 1 != num_cmds) {

                // We don't need to receive from output pipe, so discard its
                // receiving end
                close(output_pipe[RECV]);

                // Redirect the sending end of the output pipe to stdout
                int dup2_result = dup2(output_pipe[SEND], STDOUT_FILENO);

                assert_polite(dup2_result >= 0,
                              "Failed to reassign stdout to the output pipe")
            }

            //****************************************
            //----EXECUTION----------------------------
            if(execvp(exe_string, argv) < 0 ){
                printf("\n******************************\n");
                printf("!!!Error: didn't execute the given program!!!\n");
                perror(exe_string);
                exit(EXIT_FAILURE);
            }

            exit(EXIT_SUCCESS);

            //---------------------------------------
        }else if(my_pid < 0){
            printf("\n******************************\n");
            printf("Error: problem with fork\n");
            perror("error");
            exit(EXIT_FAILURE);
        }
    }
    if(output_pipe[SEND]!=-1)
        close(output_pipe[SEND]);
    if(output_pipe[RECV]!=-1)
        close(output_pipe[RECV]);

    for (int i = 0; i < num_cmds; ++i)
        wait(NULL);
}