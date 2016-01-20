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

		// Get the input command
		char* temp = strdup(user_input);
		char* input_cmd = strsep(&temp, " ");
		printf ("User input: %s, Input command: %s\n", user_input, input_cmd);		
		
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
			if(execvp(input_cmd, user_input) < 0){
				perror("Exec failed: invalid command or arguments\n");
			} 
		}

	}
}

