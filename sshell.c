#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> 
#include <sys/wait.h> 
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define CMDLINE_MAX 512  
#define TOKEN_MAX 32 
#define ARG_MAX 17
#define PIPE_MAX 4
#define ALPHABET_MAX 26

// holds possible string for each alphabet replaced by a string with set 
struct alphabet
{
  char set[TOKEN_MAX];
}; 

//each command has the main command, its arguments and possibly a file to redirect the command to 
struct command
{
        char *cmd;
        char *args[ARG_MAX];
        char x[TOKEN_MAX];
        char file[TOKEN_MAX];
};


//takes in raw input from user that includes pipes unparsed
struct std_in
{
        char input[CMDLINE_MAX];
        struct command split[PIPE_MAX];
};


void pipeline (struct command p[], int num_com,char *line){ 
       	num_com++;//number of commands were counted from zero instead of one 
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
      		// replace  stdout with write part of 1st pipe

      		dup2(fd[1], STDOUT_FILENO);

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
		
			
	  
	  		dup2(fd[0], STDIN_FILENO);

	  	

	  		dup2(fd[3], STDOUT_FILENO);

	  		
			for(int i =0;i < ((num_com - 1) * 2) ;i++)
                       		close(fd[i]);
	  	


	  		execvp(p[1].args[0],p[1].args); 
			exit(2);
		}
     
      			
       		if ( num_com == 4){
               		if (fork() == 0)
                	{
                       		
				
                        	dup2(fd[2], STDIN_FILENO);
                        	
                        	dup2(fd[5], STDOUT_FILENO);
                        	
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
				dup2(fd[0], STDIN_FILENO); 
			else if(num_com == 3) 
				dup2(fd[2], STDIN_FILENO); 
			else 
				dup2(fd[4], STDIN_FILENO);

	     	//if the last command in the pipe is to be redirected	
			if(p[num_com-1].file[0] != '\0'){  
				int fp;
				if ((fp = open(p[num_com-1].file, O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR)) == -1) {
                                fprintf(stderr,"Error: cannot open output file\n");
                                return;
                              	}  
				dup2(fp,STDOUT_FILENO);	

			}	 
		
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
       

      		
	fprintf(stderr, "+ completed  '%s' [%d]",line,retval[0]);
      	for (int i = 1; i < num_com ; i++)
        	fprintf(stderr, "[%d]",retval[i]); 
	fprintf(stderr,"\n");
}  


//stores parsed file in the file array of the command object
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
      	char file[TOKEN_MAX]; 
      	memset(obj->file, '\0', sizeof(char) );	
  	if( strchr(str, ch) != NULL){
   	char *xtr = strchr(str, ch);   
	//the filename is right after the > ,so we increment it
   	xtr  = xtr +1; 
 	strcpy(file, xtr); 
       	set_file(obj,file);//function to get rid of any space character after the > till the file name is encountered 
	//> and file are not commands so after the file is stored, we want 
	//to remove them from the input before we parse through the actual command and its arguments	
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

	//if count == 16, there are too many arguments
    	if(count == 16){ 
       		return 1; 
	}	 
	
	return 0;	
}


/*seperates the input into their different commands after pipe is encountered 
 *calls parse_arg after to split the command further into its arguments 
 */
int parse_cmd(struct std_in  *obj,char str[]){
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
       //returns number of commands seperated by the pipe 
     	 return count;

}



int main(void)
{
        
	
        char line[CMDLINE_MAX];  
	// initialize  set array to ""        		
	struct alphabet x3[ALPHABET_MAX]; 
	for(int i= 0; i <ALPHABET_MAX; i++)
        	memset(x3[i].set, '\0', sizeof(char) );
	char s[100]; 
	
        while (1) {    
		
		int num_com = 0;//number of commands seperated by pipe
		int redirect=0;//counter if we have to redirect
                char *nl;
                int retval =0; 
		pid_t pid; 
		int err = 0;//counter if something went wrong in the command parser 
		char *ptr;
		struct command x2; 
		struct std_in x1;
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
                nl = strchr(line,'\n');
                if (nl)
                        *nl = '\0';
                
		strcpy(x1.input,line); 
	       	
		//check for pipe , if there are commands split call parse-cmd otherwise no pipes and we just have one command, call parse_arg
		ptr = strchr(line, '|'); 
		char *rdptr =strchr(line, '>'); 	
              if(ptr !=  NULL){  
		    	//if there is a pipe after an output redirection  
		      	if((ptr && rdptr != NULL) && ptr > rdptr){ 
				fprintf(stderr,"Error: mislocated output redirection\n");
                                continue;
			}				
			num_com = parse_cmd(&x1,x1.input);  
			//a pipe must have a minimum of two commands
		        if(num_com < 1){ 
			fprintf(stderr,"Error: missing command\n");
			continue;
			} 
			
			pipeline(x1.split,num_com,line); 
		       	continue;	
	      } 
	      else{ 
		      if(strchr(x1.input,'>') != NULL) 
			      redirect= 1;
		   err =  parse_arg(&x2,x1.input); //err is 1 if there are too many arguments
	      	} 
	      
	      	/* check for $ and replace with string in set array*/ 
	     	int arg_count =0; 
	    	int break_ =0; 
	       	while(x2.args[arg_count] != NULL && strcmp(x2.args[0], "set") ){	 
			  
			ptr = strchr(x2.args[arg_count],'$');  
		       	if(ptr != NULL){  
				int len= strlen(x2.args[arg_count]);//checks to see if the pointer to $a $b and so on only has 2 characters and not more like $jpl
                        	if(len > 2){
                                	fprintf(stderr, "Error: invalid variable name\n");
                                	break_++;
					break;
                        	}	
				ptr = ptr +1;//increments the pointer to the character after $ to check if it is an  alphabet in lowercase 
				char j = *ptr;
				if('a'<= j  && j <= 'z'){ 
                                        int k = j - 'a';  
                                        x2.args[arg_count] = x3[k].set; 
				} 
				else{ 
					fprintf(stderr, "Error: invalid variable name\n");
                        		break_++;
					break;
				}	
		       	}	
			arg_count++; 
		} 
	   	
	      	if( break_ >0)//if any error occured in the set function, loop back to the start of the shell
	      		continue;	       


	      	  /* Builtin command current Directory */
                if (!strcmp(line, "pwd")) {
                        fprintf(stdout, "%s\n", getcwd(s,100));
                        fprintf(stderr, "+ completed '%s' [%d]\n",line, retval);


                }

                /* Builtin command exit */
                if (!strcmp(line, "exit")) {
                        fprintf(stderr,"Bye...\n");
                        fprintf(stderr, "+ completed '%s' [%d]\n",line, retval);
                        break;
                }


	      	/* Builtin command change Directory */
                if (!strcmp(x1.input, "cd")) {
                       int cd =  chdir(x2.args[1]);   
		   	if( cd < 0){ 
			       	fprintf(stderr,"Error: cannot cd into directory\n");	
				fprintf(stderr, "+ completed '%s' [%d]\n",line, ++retval);
                        	continue;
			}	
                        fprintf(stderr, "+ completed '%s' [%d]\n",line, retval);
                        continue;

                } 
		
		/* Builtin command set */ 
		if (!strcmp(x1.input, "set")) { 
        		char arg[2];  
		       	if( x2.args[1] == NULL){ 
				fprintf(stderr, "Error: invalid variable name\n"); 
				fprintf(stderr, "+ completed '%s' [%d]\n",line, ++retval);
                                continue;
			}	
			int length= strlen(x2.args[1]); 
			
			if(length > 1){ 
			 	fprintf(stderr, "Error: invalid variable name\n"); 
				fprintf(stderr, "+ completed '%s' [%d]\n",line, ++retval);
                                continue;
			}				
       			strcpy(arg, x2.args[1] ); 
      			char x= arg[0]; 
      			int y = (int)x; 
      			int z = y - 97;  //97 ascii code for 'a'
 			if(0 <= z && z <= 25){   
				if(x2.args[2]== NULL){ 
				fprintf(stderr, "+ completed '%s' [%d]\n",line,retval); 
				continue;

				}
				strcpy(x3[z].set,x2.args[2]); 
			       	fprintf(stderr, "+ completed '%s' [%d]\n",line, retval); 
			} 
			else {
				fprintf(stderr, "Error: invalid variable name\n");  
				fprintf(stderr, "+ completed '%s' [%d]\n",line, ++retval);
				continue; 
			}	
		}	
	
  


		  
		
	/*non built in commands will be carried out by the exec function 
     	* the child process executes the command 
      	* the parent waits for the child to execute the process and displays its return status*/
             if(strcmp(x1.input, "pwd") && strcmp(x1.input, "cd") && strcmp(x1.input, "set") && num_com == 0 ){ 
		int fd; 

		if(err == 1) { 
			fprintf(stderr,"Error: too many process arguments\n"); 
			continue; 
		} 
	       //no command provided	
		if(x2.args[0] == NULL){
                                continue;
                 }
		if (redirect == 1){ 
			if(x2.file[0] == '\0' ){
                                fprintf(stderr,"Error: no output file\n");
                                continue;
              		} 
		       	       
	       			
			if ((fd = open(x2.file, O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR)) == -1) {
                              	fprintf(stderr,"Error: cannot open output file\n");
                              	continue;
            		}  
			//no command to redirect 
			if(x2.args[0] == NULL){
                                fprintf(stderr,"Error: missing command\n");
                                continue;
                        }
		}		
		pid = fork();
                if(pid ==0){
                 /*child*/ 
		
		if(redirect == 1){
        	
        		dup2(fd, STDOUT_FILENO); 
			close(fd);
		}
                
		execvp(x2.args[0],x2.args);
                fprintf(stderr,"Error: command not found\n");
                exit(1);
                } else if (pid >0){ 
                  /*parent*/ 
		int status; 
		waitpid(pid,&status,0); 
	        retval = WEXITSTATUS(status);	
                }  


		
                /* Regular command */
                fprintf(stderr, "+ completed '%s' [%d]\n",line, retval); 
	     } 

        }

        return EXIT_SUCCESS;
}
