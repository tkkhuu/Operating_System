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
			if(execvp(shell_input[0], shell_input) < 0){
				perror("Exec failed: invalid command or arguments\n");
			} 
		}

	}
}

