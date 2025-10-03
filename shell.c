#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_LINE 80 /* The maximum length command */
#define MAX_TOKENS 40 /* The maximum number of tokens we could have in a command */
char *history[5]; /* Array holding the previous commands */
unsigned int num_commands_executed;
void add_new_arg_to_history(char *arg);
void split_string(char *input_string, char *string_array[]);

int main(void) {
    char arguments[MAX_LINE]; 
    int should_run = 1; /* flag to determine when to exit the program */


    while (should_run) {
        printf("osh>");
        fflush(stdout);

        /* read input, break if fgets fails */
        if (!fgets(arguments, MAX_LINE, stdin)) {
            printf("Failed reading input");
            return -1;
        } 

        arguments[strcspn(arguments, "\n")] = 0; /* gets rid of newline character */
        /* We use this pointer so we can reassign what argument is passed in if "!!" is used */
        char *arguments_pointer = arguments;
        
        /* add the latest command call to history */
        latest_command:
        /* End the program when the user types exit */
        if (!strcmp("exit", arguments_pointer)) {
            should_run = 0;
            continue;
        } 
        /* Print out history when user types history*/
        else if(!strcmp("history", arguments_pointer)) {
            if (!history[0]) {
                printf("No commands in history.\n");
                continue;
            }
            for (int i = 0; i < 5; i++) {
                if (history[i]) {
                    printf("%d. %s\n", num_commands_executed - i, history[i]);
                }
            }
        }
        /* Execute the last command when user types !! */
        else if (!strcmp("!!", arguments_pointer)) {
            if (history[0]) {
                arguments_pointer = history[0]; /* our argument is now the last command executed */
                goto latest_command; /* jump back to the start*/
            } 
            else {
                printf("No commands in history.\n");
                continue;
            }
        }
        /* For any other command execute the following code */
        else {

            pid_t pid = fork();

            /* Child process */
            if (pid == 0) {
                
                char *args_array[MAX_TOKENS];
                /* This gives us an array of the command split by spaces */
                split_string(arguments_pointer, args_array);
                
                /* Execute the command in */
                execvp(args_array[0], args_array);
                
                /* This only executes if execvp fails */
                printf("Not a valid command\n");
                exit(1);
            } 
            /* Parent process */
            else {
                wait(NULL);
                add_new_arg_to_history(arguments_pointer); 
                num_commands_executed += 1;
            }
        }
    }

    /* Freeing any memory that is being held in the history array */
    for (int i = 0; i < 5; i++) {
        if (history[i]) {
            free(history[i]);
        }
    }

    return 0;
}

/* Adds a new argument to the history array by shifting all elements over */
void add_new_arg_to_history(char *arg) {
    /* We don't want to have the history or !! command in the history */
    if (!strcmp("history", arg) || !strcmp("!!", arg)) {
        return;
    }
    /* Preventing memory leaks when we shift the array elements by one */
    if (history[4]) {
        free(history[4]);
    }
    /* Shifting each element in the history array to the right */
    for (int i = 4; i > 0; i--) {
        history[i] = history[i - 1];
    } 
    /* Assigning the first element in the history array to the passed in argument */
    history[0] = strdup(arg);
}

/* This function takes in a input string and an array */
/* It splits the input string on spaces and then adds each element to the array */
/* Adds a null terminator on the end since the execvp call needs a null terminator in the array */
void split_string(char *input_string, char *string_array[]) {
    
    /* Invalid arguments */
    if (input_string == NULL || string_array == NULL) {
        return;
    }

    /* Iterate through the input string, adding elements to the array */
    int count = 0;
    char *token = strtok(input_string, " ");
    while (token != NULL && count < MAX_TOKENS - 1) {
        string_array[count] = token;
        count++;
        token = strtok(NULL, " ");
    }

    string_array[count] = NULL; /* adds the null terminator on the end */
}
