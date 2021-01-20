#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> 
#include <sys/wait.h> 
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define CMDLINE_MAX 512  

 struct command
{
        char *cmd;
        char *args[17];
        char x[32];
        char file[32];
};

struct std_in
{
        char input[CMDLINE_MAX];
        struct command split[4];
};


void pipeline (struct command p[], int num_com,char *line){ 
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
	if (pid == 0)
    {
      		// replace cat's stdout with write part of 1st pipe

      		dup2(fd[1], 1);

      		// close all pipes
      		for(int i =0;i < ((num_com - 1) * 2) ;i++) 
		       close(fd[i]);	
		execvp(p[0].args[0],p[0].args); 
		exit(1);
    } 
	else
    {
      
	if (num_com == 3 || num_com == 4){
      		if (fork() == 0)
		{
		
			
	  
	  		dup2(fd[0], 0);

	  	

	  		dup2(fd[3], 1);

	  		
			for(int i =0;i < ((num_com - 1) * 2) ;i++)
                       		close(fd[i]);
	  	


	  		execvp(p[1].args[0],p[1].args); 
			exit(2);
		}
     
      			
       		if ( num_com == 4){
               		if (fork() == 0)
                	{
                       		
				
                        	dup2(fd[2], 0);
                        	
                        	dup2(fd[5], 1);
                        	
				for(int i =0;i < ((num_com - 1) * 2) ;i++)
                       			close(fd[i]);
                        	execvp(p[2].args[0],p[2].args); 
				exit(3);
                	}

           	}

   	} 

	if (fork() == 0)
                {
                    	
			if(num_com == 2) 
				dup2(fd[0], 0); 
			else if(num_com == 3) 
				dup2(fd[2], 0); 
			else 
				dup2(fd[4], 0);

	     		
			for(int i =0;i < ((num_com - 1) * 2) ;i++)
                       		close(fd[i]);	

                        execvp(p[num_com-1].args[0],p[num_com-1].args); 
			exit(4);
                }

	} 	
	
	int retval[num_com];
	
  	
	for(int i =0;i < ((num_com - 1) * 2) ;i++)
        	close(fd[i]); 
	for (int i = 0; i < num_com ; i++){
                wait(&status); 
		retval[i] = WEXITSTATUS(status);
        }
       

      		
	fprintf(stdout, "+ completed  '%s' [%d]",line,retval[0]);
      	for (int i = 1; i < num_com ; i++)
        	fprintf(stdout, "[%d]",retval[i]); 
	fprintf(stdout,"\n");
}  

void set_file(struct command *obj,char file[]){
	char *ptr = file;
 	while( *ptr == ' ')
  	ptr++;
  	strcpy(obj->file, ptr);
}




/* splits the input using the space character to mark the end of each argument 
 * stores output file in file array if > is encountered
 */
int parse_arg(struct command *obj,char str[]){  
	char ch = '>'; 
      	char file[32];	
  	if( strchr(str, ch) != NULL){
   	char *xtr = strchr(str, ch); 
   	xtr  = xtr +1; 
 	strcpy(file, xtr); 
       	set_file(obj,file);	
	xtr--;
   	while( *xtr != '\0'){
    		*xtr = '\0';  
     		xtr++;  
   		}	 
  	}
	char delim[] = " "; 
	int count = 0;  
	char *ptr= strtok(str, delim); 
	obj->args[count]= ptr; 
	obj->cmd = obj->args[count]; 
  	ptr = strtok(NULL, delim); 
      	count++;	
  	obj->args[count]= NULL;
  
 	if(ptr != NULL){ 
		obj->args[count] = ptr ;
		while((ptr != NULL) && (count < 16)) { 
			ptr = strtok(NULL, delim); 
			count = count + 1;
			obj->args[count] = ptr ;
 		} 
   	}	  
    	if(count == 16){ 
       		return 1; 
	}	 
	
	return 0;	
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

	
        char line[CMDLINE_MAX];	
		

        while (1) {  
		int num_com = 0; 
		struct std_in x1;	 
		struct command x2; 
		int redirect=0;
                char *nl;
                int retval =0; 
		pid_t pid; 
		int err = 0; 
		

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
                        fprintf(stderr, "+ completed  '%s' [%d]\n",line, retval);
                        break;
                }	
		

		char *ptr = strchr(x1.input, '|'); 
		char *rdptr =strchr(x1.input, '>'); 	
              if(ptr !=  NULL){   
		      	if((ptr && rdptr != NULL) && ptr > rdptr){ 
				fprintf(stderr,"Error: mislocated output redirection\n");
                                continue;
			}				
			num_com = parse_cmd(&x1,x1.input); 
		        if(num_com < 1){ 
			fprintf(stderr,"Error: missing command\n");
			continue;
			}
			pipeline(x1.split,num_com,line); 
	      } 
	      else{ 
		      if(strchr(x1.input,'>') != NULL) 
			      redirect= 1;
		   err =  parse_arg(&x2,x1.input); 
	      	} 
	


		  
		
	/*non built in commands will be carried out by the exec function 
     	* the child process executes the command 
      	* the parent waits for the child to execute the process and displays its return status*/
             if(strcmp(x1.input, "pwd") && strcmp(x1.input, "cd") && num_com == 0 ){ 
		int fd;
		if(err == 1) { 
			fprintf(stderr,"Error: too many process arguments\n"); 
			continue; 
		}
		if (redirect == 1){
			if(x2.file[0] == '\0' ){
                                fprintf(stderr,"Error: no output file\n");
                                continue;
              		} 
		       	       
	       			
			if ((fd = open(x2.file, O_WRONLY | O_CREAT | O_TRUNC | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)) == -1) {
                              	fprintf(stderr,"Error: cannot open output file\n");
                              	continue;
            		}  
			if(x2.cmd == NULL){
                                fprintf(stderr,"Error: missing command\n");
                                continue;
                        }
		}		
		pid = fork();
                if(pid ==0){
                 /*child*/ 
		
		if(redirect == 1){
        	
        		dup2(fd, 1); 
			close(fd);
		}
                
		execvp(x2.cmd,x2.args);
                fprintf(stderr,"Error: command not found\n");
                exit(1);
                } else if (pid >0){ 
                  /*parent*/ 
		int status; 
		waitpid(pid,&status,0); 
	        retval = WEXITSTATUS(status);	
                }  


		
                /* Regular command */
                fprintf(stdout, "+ completed  '%s' [%d]\n",line, retval); 
	     } 

	      /* Builtin command current Directory */
                if (!strcmp(x1.input, "pwd")) {
                        fprintf(stdout, "%s\n", getcwd(x1.input,sizeof(x1.input)));
                       
                }
 		
		 /* Builtin command change Directory */
                if (!strcmp(x1.input, "cd")) {  	
				if(chdir(x1.split[0].args[1]) < 0){ 
					fprintf(stderr,"Error: cannot cd into directory\n");
                                	retval = 1;
                        }
			fprintf(stdout, "+ completed  '%s' [%d]\n",line, retval);               

                }

        }

        return EXIT_SUCCESS;
}
