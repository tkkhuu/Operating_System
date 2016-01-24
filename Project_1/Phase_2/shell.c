#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void read_stdin();

int main(int argc, char* argv[]){

	if(argc == 1){
		read_stdin();
	}
	else {
		printf("Invalid command, usage:\n");
		printf("1) To open a shell: ./shell\n");
		printf("2) To read commands from a .txt file: ./shell < file.txt\n");
	}
}

void read_stdin(){
	while(1){
		// Prompt the user to enter command inputs
		fputs("==> ", stdout);

		// Wait and read for user inputs
		char user_input[128];
		char* f_gets_status = fgets(user_input, sizeof(user_input), stdin);
		if (f_gets_status == NULL){
			exit(0);
}
		// Parse the input
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
		
		// If the built in function chdir "cd: is called
		if(shell_input[0] == NULL){
			continue;
		}		
		else if (strcmp(shell_input[0], "cd") == 0){
			if(input_counts > 3){
				perror("shell: cd error: ");
			}
			else{
				int cd_status = chdir(shell_input[1]);
				if (cd_status < 0){
					perror("shell: cd error: ");
				}
			}
		}

		// If the built in command exit is called
		else if(strcmp(shell_input[0], "exit") == 0){
			if (input_counts > 1){
				printf("exit error: Too many arguments\n");
			}
			else{
				printf("Exit shell\n");
				exit(EXIT_SUCCESS);
			}
		}

		// ================ Create a child process ==================
		// Fork a child process
		else{
			pid_t child_process = fork();

			if (child_process < 0){ // Create Child process failed !
				perror("Child process creation failed\n");
				exit(-1);
			}

			if (child_process != 0){ // In the Parent process
				wait();
			}
			
			if (child_process == 0){ // In the Child process
				// If the other Linux commands are called
				
				if(execvp(shell_input[0], shell_input) < 0){
					perror("Exec failed: invalid command or arguments\n");
				} 
				
			}
		}
	}
}
