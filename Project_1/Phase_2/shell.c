#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* get_command(char* input_string);

int main(int argc, char* argv[]){

	while(1){
		// Prompt the user to enter command inputs
		fputs("==> ", stdout);

		// Wait and read for user inputs
		char user_input[128];
		fgets(user_input, sizeof(user_input), stdin);
		
		char* shell_input[32];
		char* token;
		char* delim = " \n";

		token = strtok(user_input, delim);
		
		int input_counts = 0;
		while(token != NULL){
			shell_input[input_counts] = token;
			token = strtok(NULL, delim);
			input_counts++;
		}

		shell_input[input_counts] = NULL;
		int j;
		for(j = 0; j < input_counts + 1; j++){
			printf("shell_input[%d]: %s\n", j, shell_input[j]);
		}
						
		// ================ Create a child process ==================
		// Fork a child process
		pid_t child_process = fork();
	
		if (child_process < 0){ // Create Child process failed !
			perror("Child process creation failed\n");
			exit(-1);
		}

		if (child_process != 0){ // In the Parent process
			wait();		
		}

		if (child_process == 0){ // In the Child process
			// If the built in command chdir() "cd" is called
			if (strcmp(shell_input[0], "cd") == 0){
				if(input_counts > 3){
					perror("cd error: Too many arguments\n");
				}
				else{
					int cd_statu = chdir(shell_input[1]);
					if (cd_statu < 0){
						perror("cd failed\n");
					}
				}
			}

			// If the built in command exit is called
			else if(strcmp(shell_input[0], "exit") == 0){
				if (input_counts > 2){
					perror("exit error: Too many arguments\n");
				}
				else{
					exit(0);
				}
			}
			
			// If the other Linux commands are called
			else if(execvp(shell_input[0], shell_input) < 0){
				perror("Exec failed: invalid command or arguments\n");
			} 
		}

	}
}

