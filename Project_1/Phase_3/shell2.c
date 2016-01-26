#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/resource.h>
#include "linked_list.h"
// Function prototypes
void read_stdin();
//void print_process_statistic(struct rusage process_rusage);
// =============================== Main Function ===============================
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

// ================================ Functions ===============================
void read_stdin(){

	pid_node* lon = NULL;	

	while(1){
		// Prompt the user to enter command inputs
		fputs("==> ", stdout);

		// Wait and read for user inputs
		char user_input[128];
		char* f_gets_status = fgets(user_input, sizeof(user_input), stdin);
		if (f_gets_status == NULL){
			exit(0);
		}
		// Parse the inputi
		char is_background = 0;
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

		if(strcmp(shell_input[input_counts - 1], "&") == 0){
			shell_input[input_counts - 1] = NULL;
			is_background = 1;
		}

		shell_input[input_counts] = NULL;

		// ================ Handler for cd and exit commands ==================
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

				if (is_background){
					int fork_status;
					struct rusage child_rusage;
					add_node(&lon, child_process, &child_rusage);
					int non = count_node(lon);
					printf("Number of background process executed: %d\n", non);

					wait4(child_process, &fork_status, WNOHANG, &child_rusage);
					mark_process_done(&lon, child_process);
				}
				else {
					int fork_status;
					struct rusage child_rusage;
					wait4(child_process, &fork_status, 0, &child_rusage);
					print_process_statistic(child_rusage);
					print_all_background_process(lon);
				}
				//				int non = count_node(lon);
				//				printf("Number of background process executed: %d\n", non);
				//print_process_statistic(child_rusage);				
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
/**
void print_process_statistic(struct rusage process_rusage){
	double user_time_sec = process_rusage.ru_utime.tv_sec;
	double user_time_usec = process_rusage.ru_utime.tv_usec;
	double sys_time_sec = process_rusage.ru_stime.tv_sec;
	double sys_time_usec = process_rusage.ru_stime.tv_usec;

	printf("[The amount of user CPU time used: %f miliseconds]\n", user_time_sec*1000.0 + user_time_usec/1000.0);
	printf("[The amount of system CPU time used: %f miliseconds]\n", sys_time_sec*1000.0 + sys_time_usec/1000.0);
	printf("[The number of times the process gave up the CPU voluntarily: %ld]\n", process_rusage.ru_nvcsw);
	printf("[The number of times the process was preempted involuntarily: %ld]\n", process_rusage.ru_nivcsw);
	printf("[The number of hard page faults: %ld]\n", process_rusage.ru_majflt);
	printf("[The number of soft page faults: %ld]\n", process_rusage.ru_minflt);

}*/
