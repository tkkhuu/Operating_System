#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/resource.h>
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
	printf("execute_command[1] : %s\n", execute_cmd[0]);
	// Fork a child process
	pid_t child_process = fork();

	if (child_process < 0){ // Create Child process failed !
		perror("Child process creation failed\n");
		exit(-1);
	}

	else if (child_process != 0){ // In the Parent process
		int status;
		struct rusage child_rusage;	
		gettimeofday(&start, NULL);	
		wait3(&status, 0, &child_rusage);
		gettimeofday(&end, NULL);

		// Determine the time at the end of the function
		time_elapsed = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec)/1000000.0; // Determine time elapsed
		double user_time_sec = child_rusage.ru_utime.tv_sec;
		double user_time_usec = child_rusage.ru_utime.tv_usec;
		double system_time_sec = child_rusage.ru_stime.tv_sec;
		double system_time_usec = child_rusage.ru_stime.tv_usec;
		printf("[Wall-clock time of the command]: %.9lf seconds\n", time_elapsed);
		printf("[The amount of user CPU time used: %.9lf miliseconds]\n", user_time_sec*1000.0 + user_time_usec/1000.0);
		printf("[The amount of system CPU time used: %.9lf miliseconds]\n", system_time_sec*1000.0 + system_time_usec/1000.0);
		printf("[The number of times the process gave up the CPU voluntarily: %ld]\n", child_rusage.ru_nvcsw);
		printf("[The number of times the process was preempted involuntarily: %ld]\n", child_rusage.ru_nivcsw);
		printf("[The number of hard page faults: %ld]\n", child_rusage.ru_majflt);
		printf("[The number of soft page faults: %ld]\n", child_rusage.ru_minflt);

	}

	else if (child_process == 0){ // In the Child process
		if(execvp(argv[1], execute_cmd) < 0){
			perror("Exec failed: invalid command or arguments\n");
		} 
	}
}
