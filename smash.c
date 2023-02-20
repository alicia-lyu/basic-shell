#include <stdio.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <wait.h>
#include <sys/stat.h>
#include <fcntl.h>

void raise_error(); // To-Do
int lexer();
void print_argv();
void get_next_cmd();
int execute();
int execute_main();
void free_argv();
int loop();
int redirect();
int pipeline();

int main()
{

    ssize_t getline_ret;
    char *line;
    size_t n;

    int lexer_ret;
    char **argv = NULL;
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
        if (strcmp(line,"\n") == 0) {
            continue;
        }

        lexer_ret = lexer(line, &argv, &argc);
        if (lexer_ret == -1)
        {
            raise_error();
            continue;
        }
        printf("%d-argument input: ", argc);
        print_argv(argv, 0, argc);

        int start = 0;
        int end = 0;
        while (start < argc)
        {   
            get_next_cmd(&start, &end, argv, argc);
            // set the end of this command to NULL as an indicator 
            // for execute() that this command is over
            // argv[end]==NULL if it is already the end of the line
            if (start == end) {
                break;
            }
            if (end < argc && argv[end] != NULL)
            {
                free(argv[end]);
                argv[end] = NULL;
            }
            // exit command
            if (start < argc && strcmp(argv[start], "exit") == 0)
            {
                fflush(stdout);
                free(line);
                free_argv(&argv, argc);
                exit(0);
            }
            // command to execute
            printf("- - - - - - Execute results - - - - - -\n");
            // fflush(stdout);
            int exec_ret = execute(argv + start, end - start);
            if (exec_ret == -1) {
                raise_error();
            }
            start = end + 1;
            end = start;
        }
        printf("All commands executed.\n");
    }
    free_argv(&argv, argc);
    free(line);
}

void print_argv(char** argv, int start, int end) {
    printf("[");
    for (int i = start; i < end; i++) {
        printf("%d:\"%s\", ", i, argv[i]);
    }
    printf("]\n");
}

void get_next_cmd(int *start, int *end, char **argv, int argc)
{   
    // skip ";"s
    while (*start < argc && strcmp(argv[*start], ";") == 0)
    {
        (*start)++;
    }
    *end = *start;
    // scan till the end of the command
    while (*end < argc 
        && argv[*end] != NULL
        && strcmp(argv[*end], ";") != 0 )
    {
        (*end)++; // the end of line or next ";"
    }
    printf("- - - - - Command found. start: %d, end: %d - - - - -\n", *start, *end);
    print_argv(argv, *start, *end);
}

int execute_main(char **argv, int argc) // could do without argc?
{   
    // add loop
    // add redirect
    if (strcmp(argv[0], "cd") == 0 ) {
        if (argc != 2) {
            return -1;
        }
        if (chdir(argv[1]) != 0) {
            return -1;
        }
    } else if (strcmp(argv[0], "pwd") == 0) {
        if (argc != 1) {
            return -1;
        }
        char* cwd = getcwd(NULL, 0);
        if (cwd == NULL) {
            return -1;
        }
        printf("%s\n", cwd);
        free(cwd);
    } else {
        pid_t pid = fork();
        if (pid == -1) {
            return -1;
        } else if (pid == 0) {
            // child
            execv(argv[0], argv);
            // execv will stop at the first NULL
            // on success, ret will not be returned
            exit(-1);
        } else {
            // parent
            int status;
            wait(&status);
            if (WEXITSTATUS(status) == 255) {
                printf("Command not found.\n");
                return -1;
                // 255: command not found
                // the only status we need to output the standard error message
            }
        }
    }
    return 0;
}

int execute(char **argv, int argc) {
    loop(argv, argc);
}

int loop(char **argv, int argc) {
    int cnt = 1;
    int ret = 0;
    if (strcmp(argv[0], "loop") == 0) {
        if (argc < 2) {
            return -1;
        }
        cnt = atoi(argv[1]);
        if (cnt <= 0) {
            return -1;
        }
    } else {
        return redirect(argv, argc);
    }
    printf("loop %d times.\n", cnt);
    for (int i = 0; i < cnt; i++) {
        printf("%d-th loop.\n", i+1);
        ret = redirect(argv + 2, argc - 2);
        if (ret == -1) {
            printf("Loop aborted.\n");
            return -1;
        }
    }
}

int redirect(char **argv, int argc) {
    printf("To be redirected: ");
    print_argv(argv, 0, argc);
    int red_pos = -1;
    for (int i = 0; i < argc; i++) {
        if (strcmp(argv[i],">") == 0) {
            if (i != argc-2) { 
                // invalid syntax of redirection
                return -1;
            } else {
                red_pos = i;
                break;
            }
        }
    }
    printf("redirection position: %d\n", red_pos);
    // no redirection
    if (red_pos == -1) {
        return execute_main(argv, argc);
    }
    // redirection begins
    // add null-terminator
    // printf("Redirected to %s\n", argv[red_pos+1]);
    free(argv[red_pos]);
    argv[red_pos] = NULL;
    // change stdout stream 
    // while maintaining stderr in console
    int stdout_cpy = dup(STDOUT_FILENO);
    int fd = open(argv[red_pos+1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd == -1) {
        return -1;
    }
    dup2(fd, STDOUT_FILENO);
    int ret = execute_main(argv, argc-2);
    // restore output stream
    dup2(stdout_cpy, STDOUT_FILENO);
    close(stdout_cpy);
    close(fd);
    // restore >
    argv[red_pos] = strdup(">");
    return ret; 
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
    *args = malloc(sizeof(char **) * ((*num_args)++));
    // store NULL at the end
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
    (*args)[(*num_args)++] = NULL;
    return 0;
}

void free_argv(char*** argv, int argc) {
    for (int i = 0; i < argc; i++)
    {
        if ((*argv)[i] != NULL) {
            free((*argv)[i]);
        }
    }
    free(*argv);
}

void raise_error()
{
    char error_message[30] = "An error has occurred\n";
    write(STDERR_FILENO, error_message, strlen(error_message));
}