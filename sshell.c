#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

#define CMDLINE_MAX 512
#define TOKEN_MAX 32
#define ARG_MAX 17
#define PIPE_MAX 4
#define ALPHABET_MAX 26

/* Holds possible string for each alphabet replaced by a string with set */
struct alphabet {
    char set[TOKEN_MAX];
};

/* Each command has the main command, its arguments and possibly a file to redirect the command to */
struct command {
    char *cmd;
    char *args[ARG_MAX];
    char x[TOKEN_MAX];
    char file[TOKEN_MAX];
};

/* Stores raw input from user that includes pipes unparsed */
struct std_in {
    char input[CMDLINE_MAX];
    struct command split[PIPE_MAX];
};

void pipeline(struct command p[], int num_com, char *line) {
    // We're off by one when entering this function, therefore increment num_com first
    num_com++;

    pid_t pid;
    int fd[6];
    int status;

    pipe(fd);
    if (num_com == 3)
        pipe(fd + 2);
    if (num_com == 4)
        pipe(fd + 4);

    pid = fork();
    if (pid == 0) {
        // Replace stdout with write part of 1st pipe
        dup2(fd[1], STDOUT_FILENO);

        // Close all pipes
        for (int i = 0; i < ((num_com - 1) * 2); i++)
            close(fd[i]);

        execvp(p[0].args[0], p[0].args);
        exit(1);
    } else {
        if (num_com == 3 || num_com == 4) {
            if (fork() == 0) {
                dup2(fd[0], STDIN_FILENO);
                dup2(fd[3], STDOUT_FILENO);

                for (int i = 0; i < ((num_com - 1) * 2); i++)
                    close(fd[i]);

                execvp(p[1].args[0], p[1].args);
                exit(2);
            }
            if (num_com == 4) {
                if (fork() == 0) {
                    dup2(fd[2], STDIN_FILENO);
                    dup2(fd[5], STDOUT_FILENO);

                    for (int i = 0; i < ((num_com - 1) * 2); i++)
                        close(fd[i]);

                    execvp(p[2].args[0], p[2].args);
                    exit(3);
                }
            }
        }
        if (fork() == 0) {
            if (num_com == 2)
                dup2(fd[0], STDIN_FILENO);
            else if (num_com == 3)
                dup2(fd[2], STDIN_FILENO);
            else
                dup2(fd[4], STDIN_FILENO);

            /* If the last command in the pipe is to be redirected */
            if (p[num_com - 1].file[0] != '\0') {
                int fp;
                if ((fp = open(p[num_com - 1].file, O_WRONLY | O_TRUNC | O_CREAT,
                               S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR)) == -1) {
                    fprintf(stderr, "Error: cannot open output file\n");
                    return;
                }
                dup2(fp, STDOUT_FILENO);
            }
            for (int i = 0; i < ((num_com - 1) * 2); i++)
                close(fd[i]);

            execvp(p[num_com - 1].args[0], p[num_com - 1].args);
            exit(4);
        }
    }

    int retval[num_com];

    for (int i = 0; i < ((num_com - 1) * 2); i++)
        close(fd[i]);

    for (int i = 0; i < num_com; i++) {
        wait(&status);
        retval[i] = WEXITSTATUS(status);
    }

    fprintf(stderr, "+ completed  '%s' [%d]", line, retval[0]);
    for (int i = 1; i < num_com; i++)
        fprintf(stderr, "[%d]", retval[i]);
    fprintf(stderr, "\n");
}
 

void exit_command(int retval, char line[]){ 
 	fprintf(stderr, "Bye...\n");
      	fprintf(stderr, "+ completed '%s' [%d]\n", line, retval);
} 


void pwd_command(int retval, char line[]){   
	char cwd[100];
	fprintf(stdout, "%s\n", getcwd(cwd, sizeof(cwd)));
      	fprintf(stderr, "+ completed '%s' [%d]\n", line, retval);
} 


void set_command(struct alphabet *lowercases ,struct command *obj, int retval,char line[]){ 
  	char arg[2];
    	if (obj->args[1] == NULL) {
                fprintf(stderr, "Error: invalid variable name\n");
                fprintf(stderr, "+ completed '%s' [%d]\n", line, ++retval);
                return;
            }

            int length = strlen(obj->args[1]);
            if (length > 1) {
                fprintf(stderr, "Error: invalid variable name\n");
                fprintf(stderr, "+ completed '%s' [%d]\n", line, ++retval);
                return;
            }
            strcpy(arg, obj->args[1]);
            /* Here we compute the ASCII code to check if it's within range */
            int var_code = (int)arg[0] - 'a';
            if (0 <= var_code && var_code <= 25) {
                if (obj->args[2] == NULL) {
                    fprintf(stderr, "+ completed '%s' [%d]\n", line, retval);
                    return;
                } 
		lowercases = lowercases + var_code;
                strcpy(lowercases->set, obj->args[2]); 
		lowercases = lowercases -  var_code;
                fprintf(stderr, "+ completed '%s' [%d]\n", line, retval);
            } else {
                fprintf(stderr, "Error: invalid variable name\n");
                fprintf(stderr, "+ completed '%s' [%d]\n", line, ++retval);
                return;
            }

} 


void cd_command(struct command *obj,int retval,char line[]){ 

	int cd = chdir(obj->args[1]);
       	if (cd < 0) {
         	fprintf(stderr, "Error: cannot cd into directory\n");
                fprintf(stderr, "+ completed '%s' [%d]\n", line, ++retval);
                return;
            }
            fprintf(stderr, "+ completed '%s' [%d]\n", line, retval);
            return;
}


/* Stores parsed file in the file array of the command object */
void set_file(struct command *obj, char file[]) {
    char *ptr = file;
    while (*ptr == ' ')
        ptr++;
    strcpy(obj->file, ptr);
}

/* Splits the input using the space character to mark the end of each argument
 * Stores output file in file array if > is encountered */
int parse_arg(struct command *obj, char str[]) {
    char ch = '>';
    char file[TOKEN_MAX];
    memset(obj->file, '\0', sizeof(char));
    if (strchr(str, ch) != NULL) {
        char *xtr = strchr(str, ch);
        // The filename is right after the > and we're currently on >, so we increment here
        xtr = xtr + 1;
        strcpy(file, xtr);
        // Call helper function to isolate only the filename
        set_file(obj, file);
        xtr--;
        while (*xtr != '\0') {
            *xtr = '\0';
            xtr++;
        }
    }
    char delim[] = " ";
    int count = 0;
    char *ptr = strtok(str, delim);

    obj->args[count] = ptr;
    obj->cmd = obj->args[count];
    ptr = strtok(NULL, delim);
    count++;
    obj->args[count] = NULL;

    if (ptr != NULL) {
        obj->args[count] = ptr;
        while ((ptr != NULL) && (count < 16)) {
            ptr = strtok(NULL, delim);
            count = count + 1;
            obj->args[count] = ptr;
        }
    }
    // If count == 16, there are too many arguments
    if (count == 16) {
        return 1;
    }
    return 0;
}

/* Separates the input into their different commands after pipe is encountered
 * Calls parse_arg after to split the command further into its arguments */
int parse_cmd(struct std_in *obj, char str[]) {
    struct command *command;
    char delim[] = "|";
    char *ptr = strtok(str, delim);
    int count = 0;
    command = &obj->split[count];
    strcpy(command->x, ptr);
    ptr = strtok(NULL, delim);

    while ((ptr != NULL) && (count < 4)) {
        count = count + 1;
        command = &obj->split[count];
        strcpy(command->x, ptr);
        ptr = strtok(NULL, delim);
    }

    for (int i = 0; i <= count; i++) {
        command = &obj->split[i];
        parse_arg(command, command->x);
    }
    // Returns number of commands separated by the pipe
    return count;
}

int main(void) {
    char line[CMDLINE_MAX];
    struct alphabet lowercases[ALPHABET_MAX];

    // Initialize set array to " "
    for (int i = 0; i < ALPHABET_MAX; i++)
        memset(lowercases[i].set, '\0', sizeof(char));

   

    while (1) {
        int num_com = 0; // Number of commands separated by pipe
        int redirect = 0; // Check if need to redirect
        int retval = 0;
        int err = 0; // Check if something went wrong in the command parser
        pid_t pid;
        char *nl;
        char *ptr;
        struct command command;
        struct std_in piped_commands;

        /* Print prompt */
        printf("sshell@ucd$ ");
        fflush(stdout);

        /* Get command line */
        fgets(line, CMDLINE_MAX, stdin);

        /* Print command line if stdin is not provided by terminal */
        if (!isatty(STDIN_FILENO)) {
            printf("%s", line);
            fflush(stdout);
        }

        /* Remove trailing newline from command line */
        nl = strchr(line, '\n');
        if (nl)
            *nl = '\0';

        strcpy(piped_commands.input, line);

        /* Check for pipe, if there are commands split call parse_cmd
         * Otherwise if no pipes, call parse_arg */
        ptr = strchr(line, '|');
        char *rdptr = strchr(line, '>');
        if (ptr != NULL) {
            /* If there is a pipe after an output redirection */
            if ((ptr && rdptr != NULL) && ptr > rdptr) {
                fprintf(stderr, "Error: mislocated output redirection\n");
                continue;
            }
            num_com = parse_cmd(&piped_commands, piped_commands.input);
            /* A pipe must have a minimum of two commands */
            if (num_com < 1) {
                fprintf(stderr, "Error: missing command\n");
                continue;
            }
            pipeline(piped_commands.split, num_com, line);
            continue;
        } else {
            if (strchr(piped_commands.input, '>') != NULL)
                redirect = 1;
            /* Err is 1 if there are too many arguments */
            err = parse_arg(&command, piped_commands.input);
        }

        /* Check for $ and replace with string in set array */
        int arg_count = 0;
        int break_ = 0;

        while (command.args[arg_count] != NULL && strcmp(command.args[0], "set")) {
            ptr = strchr(command.args[arg_count], '$');
            if (ptr != NULL) {
                /* Check length of variable name */
                int len = strlen(command.args[arg_count]);
                if (len > 2) {
                    fprintf(stderr, "Error: invalid variable name\n");
                    break_++;
                    break;
                }
                /* Check the character after $ to see if it is lowercase */
                ptr = ptr + 1;
                char var_char = *ptr;
                if ('a' <= var_char && var_char <= 'z') {
                    int var_num = var_char - 'a';
                    command.args[arg_count] = lowercases[var_num].set;
                } else {
                    fprintf(stderr, "Error: invalid variable name\n");
                    break_++;
                    break;
                }
            }
            arg_count++;
        }

        /* If any error occurred in the set function, loop back to the start of the shell */
        if (break_ > 0)
            continue;

        /* Built in command current directory */
        if (!strcmp(line, "pwd")) {
            pwd_command(retval,line); 
	    continue;
        }

        /* Built in command exit */
        if (!strcmp(line, "exit")) {
            exit_command(retval,line);
            break;
        }

        /* Built in command change directory */
        if (!strcmp(piped_commands.input, "cd")) {
            cd_command(&command,retval,line);
            continue;
        }

        /* Builtin command set */
        if (!strcmp(piped_commands.input, "set")) {
       	     	set_command(lowercases,&command,retval,line);
            	continue;
            }
       

        /* Non-built in commands will be carried out by the exec function
         * The child process executes the command
         * The parent waits for the child to execute the process and displays its return status */
        if (strcmp(piped_commands.input, "pwd") && strcmp(piped_commands.input, "cd") && strcmp(piped_commands.input, "set") && num_com == 0) {
            int fd;

            if (err == 1) {
                fprintf(stderr, "Error: too many process arguments\n");
                continue;
            }
            /* No command provided	 */
            if (command.args[0] == NULL && redirect!= 1) {
                continue;
            }
            /* If we want output redirection but did not receive a file */
            if (redirect == 1) {
                if (command.file[0] == '\0') {
                    fprintf(stderr, "Error: no output file\n");
                    continue;
                }
                /* If we cannot open the file */
                if ((fd = open(command.file, O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR)) == -1) {
                    fprintf(stderr, "Error: cannot open output file\n");
                    continue;
                }
                /* If we have no command to redirect */
                if (command.args[0] == NULL) {
                    fprintf(stderr, "Error: missing command\n");
                    continue;
                }
            }
            pid = fork();
            if (pid == 0) {
                /* Child */
                if (redirect == 1) {
                    dup2(fd, STDOUT_FILENO);
                    close(fd);
                }
                execvp(command.args[0], command.args);
                /* If we reach this line after execvp(), command wasn't found */
                fprintf(stderr, "Error: command not found\n");
                exit(1);
            } else if (pid > 0) {
                /* Parent */
                int status;
                waitpid(pid, &status, 0);
                retval = WEXITSTATUS(status);
            }
            /* Regular command */
            fprintf(stderr, "+ completed '%s' [%d]\n", line, retval);
        }
    }
    return EXIT_SUCCESS;
}
