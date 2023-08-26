#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include <fcntl.h>
#include<sys/types.h>
#include<sys/wait.h>
#include <sys/stat.h>
#include <errno.h>
#include <ctype.h>
#define BUFFSIZE 16384

int pipe_index;
int count_pipe;
int count;
char *path[5]= {"/bin/","/usr/bin/","/usr/local/bin/",\
"/student/binfo/bin/",""};

void execute_pipe_commands(char **args){
	/*
	This function handles pipes. It creates a pipe with two ends.
	One end is for the read from stdin file descriptor.
	The other one is for the write to stdout file descriptor.

	I have a for loop which will check all the standard paths for
	both the left pipe and right pipe

	My left pipe is an array terminating with a null pointer that holds 
	all the commands before the pipe character |

	My right pipe is an array terminating with a null pointer that holds 
	all the commands after the pipe character |

	The left and right pipe array have been extracted from the main 
	tokenizer array.
	
	*/
	pid_t pid1, pid2;
	int pipefd[2];
	char *temp;
	char *pipe_left_0, *pipe_right_0;
	char **pipe_left, **pipe_right;
	int i,j,k,l;
	int result;

	temp = calloc(BUFFSIZE, sizeof(char));

	pipe_left = malloc(BUFFSIZE * sizeof(char*));
	pipe_right = malloc(BUFFSIZE * sizeof(char*));


	for (i = 0; i < pipe_index; i++){
		pipe_left[i] = args[i];
	}

	for (j = 0; j < count - pipe_index - 1; j++){
		pipe_right[j] = args[pipe_index + 1 + j];
	}

	pipe_left_0 = pipe_left[0];
	pipe_right_0 = pipe_right[0];

	if (pipe(pipefd) == -1){
		perror("Error in creating pipe.\n");
		return;
	}

	pid1 = fork();

	if (pid1 < 0){
		perror("Error in forking pid1\n");
		return;
	}

	if (pid1 == 0){

		dup2(pipefd[1],STDOUT_FILENO);
		close(pipefd[0]);
		close(pipefd[1]);
		
		for (k = 0; k < 5; k++){
			temp = calloc(BUFFSIZE, sizeof(char));

			pipe_left[0] = pipe_left_0;
			strncpy(temp,path[k],strlen(path[k]));
			strncat(temp,pipe_left[0],strlen(pipe_left[0]));
			pipe_left[0] = temp;


			result = execv(pipe_left[0],pipe_left); 
			if (result == -1 && k == 4){
				printf("%s : Command not found\n",pipe_left_0);
			}
			else if (result < 0 ){
					int err = errno;
					fprintf(stderr,"%s: Error in PIPE : \n"\
					,strerror(err));
					perror("Error in execv\n");
			}

			free(temp);
		}
		exit(0);
	}

	pid2 = fork();

	if (pid2 < 0){
		perror("Error in forking pid2\n");
		return;
	}

	if (pid2 == 0){

		dup2(pipefd[0],STDIN_FILENO);
		close(pipefd[0]);
		close(pipefd[1]);


		for (l = 0; l < 5; l++){
			temp = calloc(BUFFSIZE, sizeof(char));

			pipe_right[0] = pipe_right_0;
			strncpy(temp,path[l],strlen(path[l]));
			strncat(temp,pipe_right[0],strlen(pipe_right[0]));
			pipe_right[0] = temp;

			result = execv(pipe_right[0],pipe_right); 
			if (result == -1 && l == 4){
				printf(" %s : Command not found\n",pipe_right_0);
			}
			else if (result < 0){
					int err = errno;
					fprintf(stderr,"%s: Error in PIPE : \n"\
					,strerror(err));
					perror("Error in execv\n");
			}
			
			free(temp);
		}
		exit(0);
			
	}
		close(pipefd[0]);
		close(pipefd[1]);
		
	
		waitpid(pid1,NULL,0);
		waitpid(pid2,NULL,0);
		free(pipe_left);
		free(pipe_right);
}


int execute_commands(char **args){
	/*
	This function will execute all the shell commands which exist\
	in the given paths in the path array.

	It iterates through all the paths and runs the command based on 
	where the executable exists.
	*/

	pid_t pid = fork();
	char *temp;
	char *args_0;
	int i;
	args_0 = args[0];


	if (pid == -1){
		perror("Error creasting fork()\n");
	}
	else if (pid == 0){
		/*This is the new child process
		 execution occurs here
		Just make the first arg with the path to it for execv */ 

		for (i = 0; i < 5; i++){
			args[0] = args_0;
			temp = calloc(BUFFSIZE, sizeof(char));

			strncpy(temp,path[i],strlen(path[i]));
			strncat(temp,args[0],strlen(args[0]));

			args[0] = temp;
			  
			if (execv(args[0],args) == -1){
				int err = errno;
				
				fprintf(stderr,"%s: Error in execv : \n",strerror(err));
				perror("Error in execute_commands\n");
			}
			free(temp);
		}
		exit(0);
	}
	else{
		wait(NULL);
	}

	return 1;
}


int execute_built_in(char **args){

	/* Built-in Commands to create
	cd - this will take you to the path given in the arg
		-by default id cd is passed without any arg it will take you to Home 
		directory

	exit - this will exit the shell.
	*/
	char *built_in_commands[2];
	int command_indicator, temp, i;

	command_indicator = 0;

	built_in_commands[0] = "exit";
	built_in_commands[1] = "cd";


	for (i=0; i < 2; i++){
		temp = memcmp(args[0],built_in_commands[i],strlen(args[0]));

		if (temp == 0 && strlen(args[0]) == strlen(built_in_commands[i])){
			command_indicator = 1 + i;
			break;
		}
	}

	/*0 is equal to exit */
	if (command_indicator == 1){
		printf("Shell Exited\n");
		exit(0);
	}

	/* 1 is for cd command*/
	else if (command_indicator == 2){
		if (args[1] == NULL){
			/*For when only cd is passed without any args
			Then it does default behavior as cd $HOME */
			args[1] = getenv("HOME");
		}

		if (chdir(args[1]) == -1){
			int err = errno;
			fprintf(stderr," Error in cd : %s:\n",strerror(err));
		}
		return 1;
	}
	
	return 0;
	
}

char **shell_split_line(char *line){

	/*
	This function basically cleans the input and puts them in an array
	that array is the main tokenizer array and it collects the shell commands
	separated by space provided by the user.
	*/

	int i, size;
	char **tokenized_array;
	char *item;

	i = 0;
	size = BUFFSIZE;
	tokenized_array = malloc(size * sizeof(char*));
	

	if (!tokenized_array){
		perror("Malloc Failed\n");
		exit(EXIT_FAILURE);
	}

	item = strtok(line," \r\t\n");
	while (item != NULL){

		if (strncmp(item,"|",1) == 0){
			count_pipe++;
			pipe_index = i; 
		}
		tokenized_array[i] = item;
		i++;
		count++;

		item = strtok(NULL," \n");
	}

	tokenized_array[i] = NULL;

	return tokenized_array;

}


char *shell_read_line(void){
	/*
	This is the main function where the reading happens. It takes input in a
	line using fgets() which is then parsed and then sent to be put in the 
	tokenizer array.
	*/

	char *line;
	int buffer_size;
	int item_count;
	buffer_size = BUFFSIZE;
	item_count = 0;

	line = calloc(buffer_size,sizeof(char));

	if (fgets(line,buffer_size,stdin) == NULL){
		perror("Error occured in fgets().\n");
	}
	item_count++;

	return line;
}

char *remove_extra_space(char *str){
	/*
	It removes all the extra spaces, tabs from user input and makes shell
	input more robust with unexpected user input.
	*/
	int i,j;
	char *lineC;

	lineC = calloc(BUFFSIZE,sizeof(char));

	i =0;
	j =0;

	while(str[i] != '\0'){

		if(str[i] == ' ' || str[i] == '\t'){

			if (i != 0){

			lineC[j] = ' ';
			j++;
			}

			while(str[i] == ' ' || str[i] == '\t'){
				i++;
			}
		}
		lineC[j] = str[i];
	
		i++;
		j++;
	}

	lineC[j] = '\0';

	return lineC;
}

void shell_loop(){

	/*
	This is where infinite input loop is being run. It keeps asking for user 
	input and based on the input further steps are being taken.

	It also prints the current working directory on each user input prompt.
	*/
	char cwd[1024];
	
	char *line;
	char *lineC;
	char **args;

	do {
		
		getcwd(cwd,sizeof(cwd));
		printf("Dir:%s$",cwd);

		line = shell_read_line();

		lineC = remove_extra_space(line);

		if (lineC[0] != '\n'){
			
			args = shell_split_line(lineC);

			if (count_pipe != 0){

				if (count_pipe == 1){
					execute_pipe_commands(args);
				}
				else{
					printf("More than one pipe present.\n");
					printf("Try again with one pipe.\n");
				}		
			}

			else{

				if (execute_built_in(args) == 0){
					/*
					exceute them as normal commands*/
					execute_commands(args);
				}
			
			}

			
		/*	for (i = 0; i < BUFFSIZE;i++){
				if (args[i] == NULL){
					break;
				}
				printf("Here is the %d tokenized array %s\n",i,args[i]);
							
			}*/
			free(args);
		}
	
		/* Here, we initialize some global variables like: */
		count = 0;
		count_pipe = 0;
		free(line);
		free(lineC);


	}while (1);
}

int main(){

	/*
	The main function which calls the shell loop on running the program.

	*/
	shell_loop();
	return 0;
}