#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define CMDLINE_MAX 512

/* every input can be organized into  its 
 * command 
 * arguments 
 */
struct command {
  char input[CMDLINE_MAX];
  char *cmd; 
/* maximum of 16 arguments with an extra space for the NULL argument*/
  char *args[17];
};


/* splits the input using the space character to mark the end of each argument 
 */
void parse_arg(struct command *obj, char *str) {
  char delim[] = " ";
  int count = 1;
  char *ptr = strtok(NULL, delim);
  obj->args[0] = obj->cmd;
  obj->args[1] = "";

  if (ptr != NULL) {
    obj->args[1] = ptr;
    while ((ptr != NULL) && (count < 16)) {
      ptr = strtok(NULL, delim);
      count = count + 1;
      obj->args[count] = ptr;
    }
  }  
  obj->args[count + 1] = NULL;
}

/*Extract the command from entered input and stores it  
 * calls on the next function to seperate each argument of the command 
 * splits the input using the space character to mark the end of each argument
 */
void parse_cmd(struct command *obj, char *str) {
  char delim[] = " ";
  obj->cmd = strtok(str, delim);
  parse_arg(obj, str);
}

int main(void) {
  /*char cmd[CMDLINE_MAX];
  char *args[] = {"", NULL}; */

  struct command x1;
  char line[CMDLINE_MAX];

  while (1) {
    char *nl;
    int retval;
    pid_t pid;

    /* Print prompt */
    printf("sshell$ ");
    fflush(stdout);

    /* Get command line */
    fgets(x1.input, CMDLINE_MAX, stdin);


    /* Print command line if stdin is not provided by terminal */
    if (!isatty(STDIN_FILENO)) {
      printf("%s", x1.input);
      fflush(stdout);
    }

    /* Remove trailing newline from command line */
    nl = strchr(x1.input, '\n');
    if (nl)
      *nl = '\0';

    strcpy(line, x1.input);
    parse_cmd(&x1, x1.input);


    /* Builtin exit command */
    if (!strcmp(x1.input, "exit")) {
      fprintf(stderr, "Bye...\n"); 
      fprintf(stdout, "Return status value for '%s': %d\n",
            line, 0);
      break;
    }
     /* non built in commands will be carried out by the exec function 
      * the child process executes the command 
      * the parent waits for the child to execute the process and displays its return status*/  
    if(strcmp(x1.input, "pwd") && strcmp(x1.input, "cd") ){
    pid = fork();
    if (pid == 0) {
      /*child*/
      execvp(x1.cmd, x1.args);
      perror("execvp");
      exit(1);
    } else if (pid > 0) {
      /*parent*/
      int status;
      waitpid(pid, &status, 0);
      retval = WEXITSTATUS(status);
    }
  
      /* Regular command */
      fprintf(stdout, "Return status value for '%s': %d\n",
      line, retval);
 
    
  }

	/* Builtin command current Directory */
	if (!strcmp(x1.input, "pwd")) {
             fprintf(stdout, "%s\n", getcwd(x1.input,sizeof(x1.input)));
	}

        /* Builtin command change Directory */
        if (!strcmp(x1.input, "cd")) {
             if(x1.args[1] =="")
                  chdir("..");
             else
                  chdir(x1.args[1]); 
	}
  }

  return EXIT_SUCCESS;
}
