#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

/*
 * Global variables
 */
char **env_paths = NULL;
int env_path_size = 0;
char error_message[30] = "An error has occurred\n";
int redirection_index = -1; // check if redirection is present
int redirection_count = 0;
int status = 1;
int is_parallel = -1;

/*
 * Function decalarations
 */
void internal_exec(char *line);
void exit_exec(char **args);
void cd_exec(char **args);
void path_exec(char **args);
char **getArgs(char *line);
int parallel_cmd_check(char *line);
void parallel_exec(char *line);
char *redirection_check(char *line);
void initialize_env_paths();


//Execution starts here
int main(int argc, char *argv[])
{

    initialize_env_paths();
    if (argc > 2)
    {
        // printf("11");
        write(STDERR_FILENO, error_message, strlen(error_message));
        exit(1);
    }

    if (argc == 1)
    {

        char *line = NULL;
        size_t allocated = 0;

        while (status == 1)
        {

            redirection_index = -1;
            // interactive mode
            printf("\ndash> ");

            // get input and check for error
            if (getline(&line, &allocated, stdin) == -1)
            {
                write(STDERR_FILENO, error_message, strlen(error_message));
                continue;
            }

            line[strcspn(line, "\n")] = 0;
            // check if parallel commands are present
            int ps = parallel_cmd_check(line);

            if (ps == 0)
            {
                parallel_exec(line);
            }
            else
            {
                internal_exec(line);
            }
        }
    }

    else if (argc == 2)
    {
        // batch mode
        char *line = NULL;
        size_t allocated = 0;
        FILE *batch_file;
        batch_file = fopen(argv[1], "r");

        if (batch_file == NULL)
        {
            write(STDERR_FILENO, error_message, strlen(error_message));
            exit(1);
        }

        while (getline(&line, &allocated, batch_file) != -1)
        {
            line[strcspn(line, "\n")] = 0;

            int ps = parallel_cmd_check(line);

            if (ps == 0)
            {
                parallel_exec(line);
            }
            else
            {
                internal_exec(line);
            }
        }

        fclose(batch_file);
    }

    return 0;
}

/*
 * Check if & is pressent in input line
 * returns integer on which we execute parallel command
 */
int parallel_cmd_check(char *line)
{

    // check if parallel operator is present in the input line
    if (strstr(line, "&") != NULL)
    {
        is_parallel = 1;
        return 0;
    }
    return -1;
}

/*
 * Function to perform parallel execution of commands. Controls flow of parallel command execution
 * Void function
 */
void parallel_exec(char *line)
{


    int command_count = 0;
    char *parallel_instance = strstr(line, "&");
    if (parallel_instance - line == 0)
    {
        is_parallel = -1;
        write(STDERR_FILENO, error_message, strlen(error_message));
        return;
    }

    char *token;
    char *saveptr;
    const char delimiter[] = "&";
    token = strtok_r(line, delimiter, &saveptr);

    while (token != NULL)
    {
        internal_exec(token);
        command_count ++;
        token = strtok_r(NULL, delimiter, &saveptr);
    }
    is_parallel = -1;
    while (command_count > 0){
        wait(NULL);
        command_count --;
    }
}

/*
 * Check if redirection is present
 */
char *redirection_check(char *line)
{

    char *redirection_instance = strstr(line, ">");

    if (redirection_instance - line == 0)
    {
        redirection_index = -2;
        return NULL;
    }
    // check if > is present at all
    if (redirection_instance == NULL)
    {
        redirection_index = -1;
        return NULL;
    }

    char *second_substring = redirection_instance + 1;
    char *second_redirection = strstr(second_substring, ">");

    if (second_redirection != NULL)
    {
        redirection_index = -2;
        return NULL;
    }

    char **file_args = getArgs(second_substring);

    if (file_args == NULL || file_args[0] == NULL)
    {
        redirection_index = -2;
        return NULL;
    }

    if (file_args[1] != NULL)
    {
        redirection_index = -2;
        return NULL;
    }
             // get index
    redirection_index = redirection_instance - line;
    char *filename = strdup(file_args[0]);
    return filename;
}

/*
 *Function to execute specified internal commands and handle any external command that may be given as input.
 *void function
 */
void internal_exec(char *line)
{

    char *output_file = redirection_check(line);

    if (redirection_index == -2)
    {
        redirection_index = -1;
        write(STDERR_FILENO, error_message, strlen(error_message));
        return;
    }

    char *command_part = strdup(line);
    if (redirection_index != -1)
    {
        command_part[redirection_index] = '\0';
    }

    char **args = getArgs(command_part);
    free(command_part);

    if (args == NULL)
    {
        return;
    }

    int stdout_copy = -1;
    int stderr_copy = -1;
    int file_desc = -1;
    if (redirection_index != -1 && output_file != NULL)
    {
        file_desc = open(output_file, O_CREAT | O_WRONLY | O_TRUNC, 0644);
        if (file_desc == -1)
        {
            write(STDERR_FILENO, error_message, strlen(error_message));
            return;
        }

        stdout_copy = dup(STDOUT_FILENO);
        stderr_copy = dup(STDERR_FILENO);
        dup2(file_desc, STDOUT_FILENO);
        dup2(file_desc, STDERR_FILENO);
        close(file_desc);
    }
    if (strcmp(args[0], "exit") == 0)
    {
        exit_exec(args);
        return;
    }

    if (strcmp(args[0], "cd") == 0)
    {
        cd_exec(args);
        return;
    }

    if (strcmp(args[0], "path") == 0)
    {
        path_exec(args);
        return;
    }

    int rc = fork();

    if (rc < 0)
    {
        write(STDERR_FILENO, error_message, strlen(error_message));
    }
    else if (rc == 0)
    {

        char *full_path = NULL;
        int i = 0;
        for (i = 0; i <= env_path_size; i++)
        {
            size_t path_len = strlen(env_paths[i]) + strlen(args[0]) + 2;
            full_path = malloc(path_len);

            strcpy(full_path, env_paths[i]);
            strcat(full_path, "/");
            strcat(full_path, args[0]);

            if (access(full_path, X_OK) == 0)
            {
                // full_path = temp_path;
                break;
            }
        }
        // iterate over env_paths, find if arg[0] is in any of them uasing access(), append to the path

        if (full_path != NULL)
        {
            int s = execv(full_path, args);

            if (s < 0)
            {
                write(STDERR_FILENO, error_message, strlen(error_message));
                free(full_path);
            }
        }
    }
    else
    {
        if (is_parallel == -1)
        {
            int wc = wait(NULL);
            if (wc < 0)
            {
                write(STDERR_FILENO, error_message, strlen(error_message));
            }
        }
    }

    if (redirection_index != -1)
    {
        dup2(stdout_copy, STDOUT_FILENO);
        dup2(stderr_copy, STDERR_FILENO);
        close(stdout_copy);
        close(stderr_copy);
    }

    redirection_index = -1;
    redirection_count = 0;
}

/*
 *Exit command execution function. Exitting from dash program
 */
void exit_exec(char **args)
{

    if (args[1] != NULL)
    {
        // printf("10");
        write(STDERR_FILENO, error_message, strlen(error_message));
        status = 1;
        return;
    }
    exit(0);
    status = -1;
    return;
}

/*
 * cd command execution.
 */
void cd_exec(char **args)
{

    if (args[1] == NULL || args[2] != NULL)
    {
        write(STDERR_FILENO, error_message, strlen(error_message));
        return;
    }

    if (chdir(args[1]) == -1)
    {
        write(STDERR_FILENO, error_message, strlen(error_message));
    }
}

/*
 * path command execution.
 */
void path_exec(char **args)
{
    int i;
    for (i = 0; i < env_path_size; i++)
    {
        free(env_paths[i]);
    }
    free(env_paths);

    if (args[1] == NULL)
    {
        env_paths = NULL;
        env_path_size = 0;
        return;
    }

    // free current paths

    env_path_size = 0;

    while (args[env_path_size + 1] != NULL)
    {
        env_path_size++;
    }

    env_paths = malloc(env_path_size * sizeof(char *));

    // int j=0;
    for (i = 0; args[i] != NULL; i++)
    {
        env_paths[i] = strdup(args[i + 1]);
    }
}

/*
 * Function to tokenize and get back the args from the given input.
 * return char** separated by whitespace
 */
char **getArgs(char *line)
{

    int allocated_size = 10;
    char **args = malloc(allocated_size * sizeof(char *));
    char *token;
    char *saveptr;
    const char delimiter[] = " ";
    int index = 0;
    token = strtok_r(line, delimiter, &saveptr);

    // counter and check if size exceeded, if yes use realloc
    while (token != NULL)
    {

        if (index > allocated_size - 1)
        {
            allocated_size += 10;
            args = realloc(args, allocated_size * sizeof(char *));
        }

        if (strlen(token) > 0)
        {
            args[index] = malloc((strlen(token) + 1) * sizeof(char));
            strcpy(args[index], token);
            index++;
        }
        token = strtok_r(NULL, delimiter, &saveptr);
    }

    args[index] = NULL;

    if (index == 0)
    {
        free(args);
        return NULL;
    }
    return args;
}

/*
 * Function to set up env_paths
 */
void initialize_env_paths()
{
    env_paths = malloc(sizeof(char *));
    env_paths[0] = strdup("/bin");
    env_path_size = 1;
}
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 