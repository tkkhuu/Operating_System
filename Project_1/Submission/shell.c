#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/resource.h>

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
		// Define variables to determine wall_clock time
		struct timeval start, end;
		double time_elapsed;

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
				struct rusage usage;
				gettimeofday(&start, NULL);
				int cd_status = chdir(shell_input[1]);
				if (cd_status < 0){
					perror("shell: cd error: ");
				}
				getrusage(RUSAGE_SELF, &usage);

				gettimeofday(&end, NULL);

				// Determine the time at the end of the function
				time_elapsed = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec)/1000000.0; // Determine time elapsed
				double user_time_sec = usage.ru_utime.tv_sec;
				double user_time_usec = usage.ru_utime.tv_usec;
				double system_time_sec = usage.ru_stime.tv_sec;
				double system_time_usec = usage.ru_stime.tv_usec;
				printf("User input:");
				int k;
				for(k=0; k<input_counts; k++){
					printf(" %s", shell_input[k]);
				}
				printf("\n");
				printf("[Wall-clock time of the command]: %.9lf seconds\n", time_elapsed);
				printf("[The amount of user CPU time used: %.9lf miliseconds]\n", user_time_sec*1000.0 + user_time_usec/1000.0);
				printf("[The amount of system CPU time used: %.9lf miliseconds]\n", system_time_sec*1000.0 + system_time_usec/1000.0);
				printf("[The number of times the process gave up the CPU voluntarily: %ld]\n", usage.ru_nvcsw);
				printf("[The number of times the process was preempted involuntarily: %ld]\n", usage.ru_nivcsw);
				printf("[The number of hard page faults: %ld]\n", usage.ru_majflt);
				printf("[The number of soft page faults: %ld]\n", usage.ru_minflt);
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
				printf("User input:");
				int k;
				for(k=0; k<input_counts; k++){
					printf(" %s", shell_input[k]);
				}
				printf("\n");
				printf("[Wall-clock time of the command]: %.9lf seconds\n", time_elapsed);
				printf("[The amount of user CPU time used: %.9lf miliseconds]\n", user_time_sec*1000.0 + user_time_usec/1000.0);
				printf("[The amount of system CPU time used: %.9lf miliseconds]\n", system_time_sec*1000.0 + system_time_usec/1000.0);
				printf("[The number of times the process gave up the CPU voluntarily: %ld]\n", child_rusage.ru_nvcsw);
				printf("[The number of times the process was preempted involuntarily: %ld]\n", child_rusage.ru_nivcsw);
				printf("[The number of hard page faults: %ld]\n", child_rusage.ru_majflt);
				printf("[The number of soft page faults: %ld]\n", child_rusage.ru_minflt);

			}

			if (child_process == 0){ // In the Child process
				// If the other Linux commands are called

				if(execvp(shell_input[0], shell_input) < 0){
					perror("Exec failed: invalid command or arguments");
					exit(EXIT_FAILURE);
				} 

			}
		}
	}
}

