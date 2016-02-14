#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/time.h>
#include <time.h>

#include "kitchen.h"

#define N 30 // Number of orders
#define CHEF_NUM 3
#define TRUE 1

void chef(int *chef_id);
void enter_station(int *chef_id, recipe *current_recipe, int order_number);
void leave_station(int *chef_id, recipe *current_recipe, int order_number);
void perform_step (int *chef_id, recipe *current_recipe, int order_number);

int current_order = 0;

int chef_state[3] = {IDLE, IDLE, IDLE}; /** The state of each chef. */

sem_t kitchen[4];                       /** A semaphore to keep control access for each station. */

sem_t state_mutex;                      /** A semaphore to keep control access to the state of each chef. */

sem_t lor_mutex;                        /** A semaphore to keep control access to the list of order. */

recipe orders[N];                       /** List of orders. */

/** ============================= Main function ============================= */

int main (int argc, char* argv[]){
    
    /** Define 3 threads for 3 chefs. */
    pthread_t chef_1, chef_2, chef_3;
    
    int chef_id_1 = 1; int chef_id_2 = 2; int chef_id_3 = 3;
    
    printf("Initializing orders semaphore ...\n");
    
    sem_init(&lor_mutex, 0, 1);
    
    sem_init(&state_mutex, 0, 1);
    
    printf("Initializing kitchen semaphores ...\n");
    
    sem_init(&kitchen[0], 0, 1);
    sem_init(&kitchen[1], 0, 1);
    sem_init(&kitchen[2], 0, 1);
    sem_init(&kitchen[3], 0, 1);
    
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

/** ============================= Chef Actions ============================= */
/**
 * Function to enter the stations, which is the critical regions.
 */
void enter_station(int *chef_id, recipe *current_recipe, int order_number){
    
    printf("#DEBUG enter station: chef %d", *chef_id);

	int step_to_perform = current_recipe->steps[current_recipe->next_action].action; // Next step to be performed: will either be PREP, STOVE, OVEN or SINK
    
    printf("Chef %d is waiting for %s\n", *chef_id, get_station_name(step_to_perform));
	
    sem_wait(&state_mutex);

	printf("Chef %d is in %s, waiting to enter station %s, to work on order %d, recipe %d, step %d\n", *chef_id, get_station_name(chef_state[*chef_id - 1]), get_station_name(step_to_perform), current_recipe->recipe_type, order_number, step_to_perform);
	
	sem_wait(&kitchen[step_to_perform]); // If the part of this kitchen is being used, sleep	

	if (chef_state[*chef_id - 1] != -1) {
		sem_post(&kitchen[chef_state[*chef_id - 1]]);
	}

	chef_state[*chef_id - 1] = step_to_perform;

	printf("Chef 1 is in %s, chef 2 is in %s, chef 3 is in %s\n", get_station_name(chef_state[0]), get_station_name(chef_state[1]), get_station_name(chef_state[2]));

	sem_post(&state_mutex);

}

/**
 * Function to leave the critical region
 */
void leave_station(int *chef_id, recipe *current_recipe, int order_number){

	int step_finished = current_recipe->steps[current_recipe->next_action].action; // Get the step that the chef just finished

	//printf("Chef %d finished step %s of order %d, is leaving station %s\n", *chef_id, get_station_name(step_finished), order_number, get_station_name(step_finished));

	// Move to next step on the recipe steps, if no more step, mark the recipe as finished.
	if (current_recipe->next_action == current_recipe->num_action) {
		current_recipe->is_done = 1;
		//printf("Order %d is finished by chef %d\n", order_number, *chef_id);
	} else {
		current_recipe->next_action++;
	}
	
	// printf("Chef %d needs to leave station %s\n", *chef_id, get_station_name(step_finished));

	
}

/**
 * Function to perform the step
 * Takes in a recipe
 */
void perform_step (int *chef_id, recipe *current_recipe, int order_number) {

	int curr_station = current_recipe->steps[current_recipe->next_action].action;

	int time_taken = current_recipe->steps[current_recipe->next_action].time_period;

	struct timeval start, end;

	int time_elapsed = 0;

	gettimeofday(&start, NULL);

	//printf("Chef %d is working in station %s\n", *chef_id, get_station_name(curr_station));

	while ( time_elapsed < time_taken) {

		gettimeofday(&end, NULL);

		time_elapsed = (int)(end.tv_sec - start.tv_sec);

	}

}

/** ============================= Chef thread. ============================= */

/**
 * The main thread chef, takes in the number to identify that chef and a pointer to the recipe that the chef will work on.
 */
void chef(int *chef_id){

	recipe *current_recipe = NULL;

	int order_num = 0;

	while (TRUE) {

		/** If the chef is not working on any order, request an order. */
		if(current_recipe == NULL){

			//printf("Chef %d is getting an order\n", *chef_id);

			sem_wait(&lor_mutex);

			current_recipe = next_order(orders, &current_order, N);

			order_num = current_order + 1;

			sem_post(&lor_mutex);

			printf("Chef %d got order %d, recipe %d\n", *chef_id, order_num, current_recipe->recipe_type);

		} else { /** Otherwise continue on the current order. */

			if (current_recipe->is_done == 0) {

				printf("Chef %d is working on order %d, recipe %d\n", *chef_id, order_num, current_recipe->recipe_type);

				enter_station(chef_id, current_recipe, order_num);

				perform_step(chef_id, current_recipe, order_num);

				leave_station(chef_id, current_recipe, order_num);

			} else if (current_recipe->is_done == 1){

				printf(" ================ Chef %d finished order %d recipe %d ================\n", *chef_id, order_num, current_recipe->recipe_type);

				current_recipe = NULL;

			}
		}

	}

}

/**
 * A situation when deadlock happens is when 3 chefs are waiting to enter each other's part of the kitchen, for example chef 1 in stove, chef 2 in oven and chef 3 in sink.
 * Chef 1 is waiting to enter oven, chef 2 is waiting to enter sink and chef 3 is waiting to enter stove.
 */


