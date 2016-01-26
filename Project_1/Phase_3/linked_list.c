#include "linked_list.h"


void delete_node(pid_node* head, pid_node* node){

	if(head == node){
		if(head->next == NULL){
			head->is_done = -1;
			head->process_id = -1;
		}
		if(head->next != NULL){			
			head->process_id = head->next->process_id;
			head->is_done = head->next->is_done;
			head->process_rusage = head->next->process_rusage;
			head->input_cmd = head->next->input_cmd;
			pid_node* temp = head->next;
			head->next = head->next->next;
			free(temp);
			return;
		}
	}

	pid_node* cur_node = head;
	while(cur_node->next != NULL && cur_node->next != node){
		cur_node = cur_node->next;
		if(cur_node->next == NULL){
			printf("Delete node: process %d is not in list", node->process_id);
			return;
		}

		cur_node->next = cur_node->next->next;
		free(node);

		return;

	}

}

void add_node(pid_node** head, pid_t new_value){//, struct rusage usage, char* cmd){

	// Create a node from the new value
	pid_node* new_node = (pid_node*)malloc(sizeof(pid_node));

	// Update the value of the new node 
	new_node->process_id = new_value;

	//new_node->process_rusage = usage;

	new_node->input_cmd = (char*)malloc(128 * sizeof(char));

	// Make new node next's element to be the current head
	new_node->next = *head;

	new_node->is_done = 0;

	// Make head points to the new node
	*head = new_node;

	printf("head value: %d\n", new_node->process_id);	

	return;
}

int count_node(pid_node* head){

	if(head == NULL){
		return 0;
	}

	else{
		printf("Current node is list: %ld\n", (long)head->process_id);
		return 1 + count_node(head->next);
	}
}

void mark_process_done(pid_node** head, pid_t value, struct rusage p_rusage, char* cmd){

	if(head == NULL){
		printf("Error marking process: The given list is empty");
	}

	if((*head)->process_id == value){
		(*head)->is_done = 1;
		(*head)->process_rusage = p_rusage;
		(*head)->input_cmd = cmd;
		return;
	}

	else {	

		pid_node* temp = (*head)->next;

		while(temp != NULL){
			if(temp->process_id = value){
				temp->is_done = 1;
				temp->process_rusage = p_rusage;
				strcpy(temp->input_cmd, cmd);
				return;
			}
			else {
				temp = temp->next;
			}
		}
	}
}

void print_all_background_process(pid_node* head){
	if(head == NULL){
		printf("Print list of background processes: no finished background process detected\n");
	}

	pid_node* finished_processes[20];

	pid_node* temp = head;

	while(temp != NULL){
		if(temp->is_done == 1 && temp->process_id != -1){
			printf("[============ Background Process with ID %ld and command '%s' terminated ============]\n", (long)temp->process_id, temp->input_cmd);
			print_process_statistic(temp->process_rusage);
			delete_node(head, temp);
		}
		//printf("Process %d is done? %d\n", (int)temp->process_id, temp->is_done);
		temp = temp->next;
	}
}

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

}


