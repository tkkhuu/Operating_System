#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>
#include <time.h>

#include "kitchen.h"

#define N 30 // Number of orders
#define CHEF_NUM 3
#define TRUE 1
#define WAIT_TIME 17

/** Function prototypes. */

void chef(int *chef_id);
void enter_station(int *chef_id, recipe *current_recipe, int order_number);
void leave_station(int *chef_id, recipe *current_recipe, int order_number);
void perform_step (int *chef_id, recipe *current_recipe, int order_number);
int get_chef_in_station(int station);

/** Global Variables. */
int current_order = 0;

int chef_state[3] = {IDLE, IDLE, IDLE}; /** The state of each chef. */

int chef_next_state[3] = {IDLE, IDLE, IDLE};

int chef_priority[3] = {0, 0, 0};

struct timespec wait_limit;

recipe orders[N];       /** List of orders. */

/** Mutexes and Condition Variables. */

pthread_cond_t kitchen_cond[4];     /** A condition variables to keep control access for each station. */

pthread_mutex_t kitchen_mutex[4];    /** Mutexes to keep control access for each station. */

pthread_mutex_t state_mutex;        /** A semaphore to keep control access to the state of each chef. */

pthread_mutex_t next_state_mutex;

pthread_mutex_t priority_mutex;     /** Mutex to keep control access to the priority of each chef. */

pthread_mutex_t lor_mutex;          /** Mutex to keep control access to the list of order. */

/** ============================= Chef Actions ============================= */
/**
 * Function to enter the stations, which is the critical regions.
 */
void enter_station(int *chef_id, recipe *current_recipe, int order_number){

	printf("========> #DEBUG enter station: chef %d\n", *chef_id);

	// Next step to be performed: will either be PREP, STOVE, OVEN or SINK
	int step_to_perform = current_recipe->steps[current_recipe->next_action].action;

	printf("Chef %d is waiting for %s\n", *chef_id, get_station_name(step_to_perform));

	pthread_mutex_lock(&state_mutex);
	int chef_current_state = chef_state[*chef_id - 1];
	pthread_mutex_unlock(&state_mutex);

	int waited_by = -1; unsigned int potential_deadlock = 0;

	if(chef_current_state != -1){

		pthread_mutex_lock(&next_state_mutex);
		int k = 0;
		for (k = 0; k < 3; k++) {
			if (chef_next_state[k] == chef_current_state){
				waited_by = k;
			}
		}

		if (waited_by != -1) {

			if (chef_next_state[*chef_id - 1] == chef_state[waited_by]) {
				potential_deadlock = 1;
			}
		}

		pthread_mutex_unlock(&next_state_mutex);
	}
	// Check if the station is available, if not, only waits for WAIT_LIMIT seconds
	if (potential_deadlock == 1){

		pthread_mutex_lock(&priority_mutex);

		if (chef_priority[*chef_id - 1] > chef_priority[waited_by]) {

			printf("POTENTIAL DEADLOCK: Chef %d waited too long for station %s, but it has higher priority, so it continues, released station %s\n", *chef_id, get_station_name(step_to_perform), get_station_name(chef_state[*chef_id - 1]));

			pthread_mutex_unlock(&priority_mutex);

			pthread_mutex_lock(&kitchen_mutex[step_to_perform]); // If the part of this kitchen is being used, sleep

			pthread_mutex_lock(&state_mutex);

			if (chef_state[*chef_id - 1] != -1) {
				pthread_mutex_unlock(&kitchen_mutex[chef_current_state]);
				pthread_cond_signal(&kitchen_cond[chef_current_state]);
			}

			chef_state[*chef_id - 1] = step_to_perform;


			pthread_mutex_unlock(&state_mutex);

			pthread_mutex_lock(&next_state_mutex);
			chef_next_state[*chef_id - 1] = current_recipe->steps[current_recipe->next_action + 1].action;
			pthread_mutex_unlock(&next_state_mutex);

			pthread_mutex_lock(&priority_mutex);
			chef_priority[*chef_id - 1] = current_recipe->next_action;
			pthread_mutex_unlock(&priority_mutex);

		} else{

			printf("XXXXXXXXXXXX  POTENTIAL DEADLOCK, Chef %d drop order %d, released station %s\n", *chef_id, order_number, get_station_name(chef_state[*chef_id - 1]));

			pthread_mutex_unlock(&priority_mutex);

			pthread_mutex_lock(&state_mutex);

			if (chef_state[*chef_id - 1] != -1) {
				pthread_mutex_unlock(&kitchen_mutex[chef_current_state]);
				pthread_cond_signal(&kitchen_cond[chef_current_state]);
			}

			chef_state[*chef_id - 1] = -1;

			pthread_mutex_unlock(&state_mutex);

			pthread_mutex_lock(&next_state_mutex);
			chef_next_state[*chef_id - 1] = current_recipe->steps[current_recipe->next_action + 1].action;
			pthread_mutex_unlock(&next_state_mutex);

			// Resetting the order
			pthread_mutex_lock(&lor_mutex);
			current_recipe->next_action = 0;
			current_recipe->is_done = 2;
			current_recipe->in_progress = 0;
			pthread_mutex_unlock(&lor_mutex);

			// Set chef priority to be 0
			pthread_mutex_lock(&priority_mutex);
			chef_priority[*chef_id - 1] = 0;
			pthread_mutex_unlock(&priority_mutex);

		}

	} else { // If the station is available

		pthread_mutex_lock(&state_mutex);

		printf("Chef %d released station %s\n", *chef_id, get_station_name(chef_state[*chef_id - 1]));

		if (chef_state[*chef_id - 1] != -1) {
			pthread_mutex_unlock(&kitchen_mutex[chef_current_state]);
			pthread_cond_signal(&kitchen_cond[chef_current_state]);
		}

		chef_state[*chef_id - 1] = step_to_perform;

		pthread_mutex_unlock(&state_mutex);

		pthread_mutex_lock(&next_state_mutex);
		chef_next_state[*chef_id - 1] = current_recipe->steps[current_recipe->next_action + 1].action;
		pthread_mutex_unlock(&next_state_mutex);

		pthread_mutex_lock(&priority_mutex);
		chef_priority[*chef_id - 1] = current_recipe->next_action;
		pthread_mutex_unlock(&priority_mutex);
	}
}

/**
 * Function to leave the critical region
 */
void leave_station(int *chef_id, recipe *current_recipe, int order_number){

	pthread_mutex_lock(&state_mutex);

	if (chef_state[*chef_id - 1] != -1){

		int step_finished = current_recipe->steps[current_recipe->next_action].action; // Get the step that the chef just finished

		// Move to next step on the recipe steps, if no more step, mark the recipe as finished.
		if (current_recipe->next_action == current_recipe->num_action - 1) {
			current_recipe->is_done = 1;
			current_recipe->in_progress = 0;
		} else {
			current_recipe->next_action++;
		}
	}

	pthread_mutex_unlock(&state_mutex);

}

/**
 * Function to perform the step
 * Takes in a recipe
 */
void perform_step (int *chef_id, recipe *current_recipe, int order_number) {

	pthread_mutex_lock(&state_mutex);

	if(chef_state[*chef_id - 1] != -1){
		int curr_station = current_recipe->steps[current_recipe->next_action].action;

		int time_taken = current_recipe->steps[current_recipe->next_action].time_period;

		struct timeval start, end;

		int time_elapsed = 0;

		gettimeofday(&start, NULL);

		printf("Chef %d is working in station %s\n", *chef_id, get_station_name(curr_station));

		while ( time_elapsed < time_taken) {

			gettimeofday(&end, NULL);

			time_elapsed = (int)(end.tv_sec - start.tv_sec);

		}
	}

	pthread_mutex_unlock(&state_mutex);


}

/** ============================= Chef thread. ============================= */

/**
 * The main thread chef, takes in the number to identify that chef and a pointer to the recipe that the chef will work on.
 */
void chef(int *chef_id){

	recipe *current_recipe = NULL;

	int order_cursor = 0;

	int order_num = 0;

	while (TRUE) {

		/** If the chef is not working on any order, request an order. */
		if(current_recipe == NULL){

			printf("Chef %d is getting an order\n", *chef_id);

			pthread_mutex_lock(&lor_mutex);

			int k;
			printf("Chef %d checking for dropped order\n", *chef_id);
			for(k = 0; k < 30; k++){

				if(orders[k].is_done == 2){
					order_num = k + 1;
					current_recipe = &orders[k];
					orders[k].is_done = 0;
					orders[k].in_progress = 1;
					printf("Chef %d found dropped order\n", *chef_id);
					break;
				}
			}
			if(current_recipe == NULL){


				current_recipe = next_order(orders, &order_cursor, N);

				order_num = order_cursor;
			}
			pthread_mutex_unlock(&lor_mutex);

			//printf("Chef %d got order %d, recipe %d\n", *chef_id, order_num, current_recipe->recipe_type);

		} else { /** Otherwise continue on the current order. */

			if (current_recipe->is_done == 2) {
				printf("Chef %d move onto next order\n", *chef_id);
				pthread_mutex_lock(&lor_mutex);

				current_recipe = next_order(orders, &order_cursor, N);
				order_num = order_cursor;
				pthread_mutex_unlock(&lor_mutex);
			}

			else if (current_recipe->is_done == 0) {

				printf("Chef %d is working on order %d, recipe %d\n", *chef_id, order_num, current_recipe->recipe_type);

				enter_station(chef_id, current_recipe, order_num);

				perform_step(chef_id, current_recipe, order_num);

				leave_station(chef_id, current_recipe, order_num);

				pthread_mutex_lock(&priority_mutex);
				pthread_mutex_lock(&lor_mutex);
				int count = 0; int k = 0;
				printf("Chef %d: [", *chef_id);
				for(k = 0; k < 30; k++){
					if(orders[k].is_done == 1){
						count++;
						printf("Done, ");
					}
					else if(orders[k].in_progress == 1){
						printf("Progress, ");
					}
					else{
						printf("Wait, ");
					}
				}
				printf("]\nOrders done: %d\n", count);
				pthread_mutex_unlock(&lor_mutex);
				pthread_mutex_unlock(&priority_mutex);

			} else if (current_recipe->is_done == 1){

				printf(" ================ Chef %d finished order %d recipe %d ================\n", *chef_id, order_num, current_recipe->recipe_type);

				current_recipe = NULL;

			}
		}

	}

}


/** ============================= Main function ============================= */

int main (int argc, char* argv[]){

	/** Define 3 threads for 3 chefs. */
	pthread_t chef_1, chef_2, chef_3;

	int chef_id_1 = 1; int chef_id_2 = 2; int chef_id_3 = 3;

	printf("Initializing orders semaphore ...\n");

	pthread_mutex_init(&lor_mutex, 0);

	pthread_mutex_init(&state_mutex, 0);

	pthread_mutex_init(&priority_mutex, 0);

	pthread_mutex_init(&next_state_mutex, 0);

	printf("Initializing kitchen semaphores ...\n");

	pthread_mutex_init(&kitchen_mutex[0], 0);
	pthread_mutex_init(&kitchen_mutex[1], 0);
	pthread_mutex_init(&kitchen_mutex[2], 0);
	pthread_mutex_init(&kitchen_mutex[3], 0);

	pthread_cond_init(&kitchen_cond[0], 0);
	pthread_cond_init(&kitchen_cond[1], 0);
	pthread_cond_init(&kitchen_cond[2], 0);
	pthread_cond_init(&kitchen_cond[3], 0);

	printf("Creating orders ...\n");

	int init = 0;

	srand(time(NULL));

	for (init = 0; init < N; init++) {
		int rep_num = (rand() % 5) + 1;
		orders[init] = generate_recipe(rep_num);
		printf("Order number %d is recipe %d\n", init, rep_num);
	}

	printf("Creating threads ...\n");

	pthread_create (&chef_1, NULL, (void *) &chef, &chef_id_1);
	pthread_create (&chef_2, NULL, (void *) &chef, &chef_id_2);
	pthread_create (&chef_3, NULL, (void *) &chef, &chef_id_3);

	printf("Run threads ...\n");

	pthread_join(chef_1, NULL);
	pthread_join(chef_2, NULL);
	pthread_join(chef_3, NULL);

	exit(0);
}

int get_chef_in_station(int station){
	if (chef_state[0] == station) {
		return 0;
	}
	else if(chef_state[1] == station){
		return 1;
	}
	else if (chef_state[2] == station){
		return 2;
	}
	else return -1;
}
