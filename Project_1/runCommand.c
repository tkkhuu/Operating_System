#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char* argv[]){
	// Create the operation to be used in execvp	
	char* execute_cmd[argc];
	int i;
	for(i = 1; i < argc; i++){
		execute_cmd[i - 1] = argv[i];
	}

	execute_cmd[argc - 1] = NULL;

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
		if(execvp(argv[1], execute_cmd) < 0){
			perror("Exec failed: invalid command or arguments\n");
		} 
	}
}
