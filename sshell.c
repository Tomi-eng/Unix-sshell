#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> 
#include <sys/wait.h>

#define CMDLINE_MAX 512 

struct command
{  char input[CMDLINE_MAX];
   char *cmd; 
   char *args[17]; 
};



void parse_arg(struct command *obj,char *str){
  char delim[] = " "; 
  int count = 1;  
  char *ptr= strtok(NULL, delim); 
  obj->args[0]= obj->cmd;
  obj->args[1] = ""; 
  
  if(ptr != NULL){ 
	obj->args[1] = ptr ;
	while((ptr != NULL) && (count < 16)) { 
		ptr = strtok(NULL, delim); 
		count = count + 1;
		obj->args[count] = ptr ;
 	} 
      }	
	obj->args[count+1] = NULL;    
}

void parse_cmd(struct command *obj,char *str){ 
	char delim[] = " "; 
	obj->cmd= strtok(str, delim); 
	parse_arg(obj,str);
}



int main(void)
{
        /*char cmd[CMDLINE_MAX]; 
	char *args[] = {"", NULL};*/ 

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
                nl = strchr(x1.input,'\n');
                if (nl)
                        *nl = '\0';
                
		strcpy(line, x1.input); 
		parse_cmd(&x1,x1.input);


		  /* Builtin command */
                if (!strcmp(x1.input, "exit")) {
                        fprintf(stderr, "Bye...\n");
                        break;
                }


		pid = fork();
                if(pid ==0){
                 /*child*/
                execvp(x1.cmd,x1.args);
                perror("execvp");
                exit(1);
                } else if (pid >0){ 
                  /*parent*/ 
		int status; 
		waitpid(pid,&status,0); 
	        retval = WEXITSTATUS(status);	
                }  


		
                /* Regular command */
                fprintf(stdout, "Return status value for '%s': %d\n",
                        line, retval);
        }

        return EXIT_SUCCESS;
}
