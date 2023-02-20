#include <stdio.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <wait.h>

void raise_error(); // To-Do
int lexer();
void print_argv();
void get_next_cmd();
int execute();
void free_argv();

int main()
{

    ssize_t getline_ret;
    char *line = malloc(sizeof(char) * 1024);
    size_t n;

    int lexer_ret;
    char **argv;
    int argc;

    while (1)
    {
        printf("smash> ");
        fflush(stdout);

        getline_ret = getline(&line, &n, stdin);
        if (getline_ret == -1)
        {
            raise_error();
            continue;
        }

        lexer_ret = lexer(line, &argv, &argc);
        if (lexer_ret == -1)
        {
            raise_error();
            continue;
        }

        printf("%d-argument input\n", argc);
        print_argv(argv, 0, argc);

        int start = 0;
        int end = 0;
        int ret = 0;
        while (start < argc)
        {
            get_next_cmd(&start, &end, argv, argc);
            // set the end of this command to NULL as an indicator 
            // for execute() that this command is over
            // argv[end]==NULL if it is already the end of the line
            if (argv[end] != NULL) 
            {
                argv[end] == NULL;
                free(argv[end]);
            }
            // exit command
            if (strcmp(argv[start], "exit") == 0)
            {
                fflush(stdout);
                free(line);
                free_argv(argv, argc);
                return 0;
            }
            // command to execute
            printf("- - - - - - Execute results - - - - - -");
            int exec_ret = execute(argv + start, end - start);
            if (exec_ret == -1) {
                raise_error();
            }
            start = end + 1;
            end = start;
        }
        free_argv(argv, argc);
    }
}

void print_argv(char** argv, int start, int end) {
    printf("[");
    for (int i = start; i < end; i++) {
        printf("%d: %s, ", i, argv[i]);
    }
    printf("]\n");
}

void get_next_cmd(int *start, int *end, char **argv, int argc)
{
    // skip ";"s
    while (*start < argc && strcmp(argv[*start], ";") == 0)
    {
        *start += 1;
    }
    // scan till the end of the command
    while (*end < argc && strcmp(argv[*end], ";") != 0)
    {
        *end += 1; // the end of line or next ";"
    }
    printf("start: %d, end: %d\n", *start, *end);
    print_argv(argv, *start, *end);
}

int execute(char **argv, int argc)
{
    if (strcmp(argv[0], "cd") == 0 ) {
        // To-do
    } else if (strcmp(argv[0], "pwd") == 0) {
        // To-do
    } else {
        pid_t pid = fork();
        if (pid == -1) {
            return -1;
        } else if (pid == 0) {
            // child
            int ret = 0;
            ret = execv(argv[0], argv); 
            // execv will stop at the first NULL
            // on success, ret will not be returned
            if (ret == -1) {
                exit(-1);
            } else if (ret == 0) {
                printf("Child process success.");
                exit(0);
            }
        } else {
            // parent
            int status;
            wait(&status);
            if (WEXITSTATUS(status) == 255) {
                return -1; 
                // the only status we need to output the standard error message
            }
        }
    }
    return 0;
}



/// description: Takes a line and splits it into args similar to how argc
///              and argv work in main
/// line:        The line being split up. Will be mangled after completion
///              of the function<
/// args:        a pointer to an array of strings that will be filled and
///              allocated with the args from the line
/// num_args:    a point to an integer for the number of arguments in args
/// return:      returns 0 on success, and -1 on failure
int lexer(char *line, char ***args, int *num_args)
{
    *num_args = 0;
    // count number of args
    char *l = strdup(line);
    if (l == NULL)
    {
        return -1;
    }
    char *token = strtok(l, " \t\n");
    while (token != NULL)
    {
        (*num_args)++;
        token = strtok(NULL, " \t\n");
    }
    free(l);
    // split line into args
    *args = malloc(sizeof(char **) * *num_args);
    *num_args = 0;
    token = strtok(line, " \t\n");
    while (token != NULL)
    {
        char *token_copy = strdup(token);
        if (token_copy == NULL)
        {
            return -1;
        }
        (*args)[(*num_args)++] = token_copy;
        token = strtok(NULL, " \t\n");
    }
    return 0;
}

void free_argv(char** argv, int argc) {
    for (int i = 0; i < argc; i++)
    {
        if (argv[i] != NULL) {
            free(argv[i]);
        }
    }
    free(argv);
}

void raise_error()
{
    char error_message[30] = "An error has occurred\n";
    write(STDERR_FILENO, error_message, strlen(error_message));
}