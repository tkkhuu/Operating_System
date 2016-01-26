#ifndef LINKED_LIST
#define LINKED_LIST

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/resource.h>

struct pid_linked_list {
	
	pid_t process_id;
	
	struct pid_linked_list* next;

	char is_done;

	struct rusage process_rusage;

	double wall_time; // in miliseconds

	char* input_cmd;
};

typedef struct pid_linked_list pid_node;

void delete_node(pid_node* head, pid_node* node);

void add_node(pid_node** head, pid_t new_value);// struct rusage usage, char* cmd);

int count_node(pid_node* head);

int count_unfinished_node(pid_node* head);

void mark_process_done(pid_node** head, pid_t value, struct rusage p_rusage, double elapsed_time);

void print_unfinished_background_process(pid_node* head);

void print_finished_background_process(pid_node* head);

void print_process_statistic(struct rusage process_rusage);


#endif
