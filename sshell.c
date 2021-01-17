#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> 
#include <sys/wait.h>

#define CMDLINE_MAX 512  



 struct command
{ 
	char *cmd; 
	char *args[17];  
        char x[32]; 
	char *file;	
};

struct std_in
{   
	char input[CMDLINE_MAX];
	struct command split[4];  
};  




/* splits the input using the space character to mark the end of each argument 
 * stores output file in file array if > is encountered
 */
void parse_arg(struct command *obj,char *str){  
	char ch = '>';  
	if( strchr(str, ch) != NULL){
   	char *file = strchr(str, ch); 
	file  = file +1; 
	strcpy(obj->file, file);
   	memset(file, '\0', sizeof(file)); 
	}
	char delim[] = " "; 
	int count = 0;  
	char *ptr= strtok(str, delim); 
	obj->args[count]= ptr; 
	obj->cmd = obj->args[count]; 
  	ptr = strtok(NULL, delim); 
      	count++;	
  	obj->args[count]= "";
  
 	if(ptr != NULL){ 
		obj->args[count] = ptr ;
		while((ptr != NULL) && (count < 16)) { 
			ptr = strtok(NULL, delim); 
			count = count + 1;
			obj->args[count] = ptr ;
 		} 
   	}	    
}


/*seperates the input into their different commands after pipe is encountered 
 *calls parse_arg after to split the command further into its arguments 
 */
int parse_cmd(struct std_in  *obj,char *str){
        struct command *x1; 
        char delim[] = "|";
        char *ptr = strtok(str, delim);
        int count = 0;
        x1= &obj->split[count];  
        strcpy(x1->x,ptr);  
        ptr = strtok(NULL, delim);
      
        while((ptr != NULL) && (count < 4)) {
                count = count + 1;
                x1 = &obj->split[count];
                strcpy(x1->x ,ptr); 
                ptr = strtok(NULL, delim);
        } 
        
        for(int i = 0; i <=count; i++){  
           x1 = &obj->split[i];
           parse_arg( x1, x1->x);
        }
        
     	 return count;

}



int main(void)
{
        /*char cmd[CMDLINE_MAX]; 
	char *args[] = {"", NULL};*/ 

	struct std_in x1; 
        char line[CMDLINE_MAX]; 
	int num_com = 0;	

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
		/* Builtin command */
                if (!strcmp(x1.input, "exit")) {
                        fprintf(stderr, "Bye...\n");
                        break;
                }	
		
              if(strchr(x1.input, '|') !=  NULL) 
		     num_com = parse_cmd(&x1,x1.input); 
	      else 
		     parse_arg(&x1.split[0],x1.input); 
	


		  
		
	/*non built in commands will be carried out by the exec function 
     	* the child process executes the command 
      	* the parent waits for the child to execute the process and displays its return status*/
             if(strcmp(x1.input, "pwd") && strcmp(x1.input, "cd") && num_com == 0 ){
		pid = fork();
                if(pid ==0){
                 /*child*/
                execvp(x1.split[0].cmd,x1.split[0].args);
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

	      /* Builtin command current Directory */
                if (!strcmp(x1.input, "pwd")) {
                        fprintf(stdout, "%s\n", getcwd(x1.input,sizeof(x1.input)));
                       
                }
 		
		 /* Builtin command change Directory */
                if (!strcmp(x1.input, "cd")) {  	
				chdir(x1.split[0].args[1]);

                }

        }

        return EXIT_SUCCESS;
}
