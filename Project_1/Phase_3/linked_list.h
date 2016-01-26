#ifndef LINKED_LIST
#define LINKED_LIST

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/resource.h>

struct pid_linked_list {
	
	pid_t process_id;
	
	struct pid_linked_list* next;

	char is_done;

	struct rusage* process_rusage;
};

typedef struct pid_linked_list pid_node;

void delete_node(pid_node* head, pid_node* node);

void add_node(pid_node** head, pid_t new_value, struct rusage* usage);

int count_node(pid_node* head);

void mark_process_done(pid_node** head, pid_t value);

void print_all_background_process(pid_node* head);

void print_process_statistic(struct rusage process_rusage);


#endif
