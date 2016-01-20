#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char* argv[]){
	if (argc <= 1){
		perror("Not enough argument, usage: ./runCommand <command> <arguments>. \n");
	}

	// Define variables to determine wall_clock time
	struct timeval start, end;
	double time_elapsed;
	
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
	
	gettimeofday(&start, NULL); // Determine the time at the start of the execution
	if (child_process != 0){ // In the Parent process
		wait();		
	}

	if (child_process == 0){ // In the Child process
		if(execvp(argv[1], execute_cmd) < 0){
			perror("Exec failed: invalid command or arguments\n");
		} 
	}

	// Determine the time at the end of the function
	gettimeofday(&end, NULL);
	time_elapsed = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec)/1000000.0; // Determine time elapsed.

	// Print out elapsed time
	printf("Time taken to execute: %.9lf seconds\n", time_elapsed);
}
