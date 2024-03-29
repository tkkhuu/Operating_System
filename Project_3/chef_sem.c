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
int get_chef_in_station(int station);
int check_deadlock(int value);
int is_lowest_priority(int value);

int current_order = 0;

int chef_state[3] = {IDLE, IDLE, IDLE}; /** The state of each chef. */

int chef_next_state[3] = {IDLE, IDLE, IDLE};

int chef_priority[3] = {0, 0, 0};

sem_t kitchen[4];                       /** A semaphore to keep control access for each station. */

sem_t state_mutex;                      /** A semaphore to keep control access to the state of each chef. */

sem_t next_state_mutex;

sem_t priority_mutex;

sem_t lor_mutex;                        /** A semaphore to keep control access to the list of order. */

recipe orders[N];                       /** List of orders. */

struct timespec wait_limit;// = {16, 0};

/** ============================= Main function ============================= */

int main (int argc, char* argv[]){
    
    /** Define 3 threads for 3 chefs. */
    pthread_t chef_1, chef_2, chef_3;
    
    int chef_id_1 = 1; int chef_id_2 = 2; int chef_id_3 = 3;
    
    printf("Initializing orders semaphore ...\n");
    
    sem_init(&lor_mutex, 0, 1);
    
    sem_init(&state_mutex, 0, 1);
    
    sem_init(&priority_mutex, 0, 1);
    
    sem_init(&next_state_mutex, 0, 1);
    
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
    
    printf("========> #DEBUG enter station: chef %d\n", *chef_id);
    
    // Next step to be performed: will either be PREP, STOVE, OVEN or SINK
    int step_to_perform = current_recipe->steps[current_recipe->next_action].action;
    
    printf("Chef %d is waiting for %s\n", *chef_id, get_station_name(step_to_perform));
    
    sem_wait(&state_mutex);
    int chef_current_state = chef_state[*chef_id - 1];
    
    
    int waited_by = -1; unsigned int potential_deadlock = 0;
    
    if(chef_current_state != IDLE){
        
        sem_wait(&next_state_mutex);
        int k = 0; int i = 0;
        for (k = 0; k < 3; k++) {
            if (chef_next_state[k] == chef_current_state){
                waited_by = k;
            }
        }
        
        if (waited_by != IDLE) {
            
            if (chef_next_state[*chef_id - 1] == chef_state[waited_by]) {
				printf("Chef %d normal deadlock detected \n", *chef_id);                
				potential_deadlock = 1;
            }

			else if (check_deadlock(chef_state[0]) && check_deadlock(chef_state[1]) && check_deadlock(chef_state[2])) {
				printf("Chef %d Circle deadlock detected \n", *chef_id);				
				potential_deadlock = 2;
			}
        }
        
        sem_post(&next_state_mutex);
		
    }
    sem_post(&state_mutex);

	if (potential_deadlock == 2) { // If potential deadlock detected

		sem_wait(&priority_mutex);

		// check the priority of the current chef, if higher than waited_by chef, it gets to continue
		if (is_lowest_priority(*chef_id - 1) == 1) {

			printf("XXXXXXXXXXXX  CIRCLE DEADLOCK, Chef %d drop order %d, released station %s\n", *chef_id, order_number, get_station_name(chef_state[*chef_id - 1]));

			sem_post(&priority_mutex);

			sem_wait(&state_mutex);

			int temp_cur = chef_current_state;
			chef_state[*chef_id - 1] = IDLE;
			if (temp_cur != IDLE) {
				sem_post(&kitchen[temp_cur]);
			}

			sem_post(&state_mutex);

			sem_wait(&next_state_mutex);
			chef_next_state[*chef_id - 1] = IDLE;
			sem_post(&next_state_mutex);

			// Resetting the order
			sem_wait(&lor_mutex);
			current_recipe->next_action = 0;
			current_recipe->is_done = 2;
			current_recipe->in_progress = 0;
			sem_post(&lor_mutex);

			// Set chef priority to be 0
			sem_wait(&priority_mutex);
			chef_priority[*chef_id - 1] = 0;
			sem_post(&priority_mutex);



		} else{ // If the current chef has lower priority, return the order and go get a new order.

			printf("CIRCLE DEADLOCK: Chef %d waited too long for station %s, but it has higher priority, so it continues, released station %s\n", *chef_id, get_station_name(step_to_perform), get_station_name(chef_state[*chef_id - 1]));

			sem_post(&priority_mutex);
			sem_wait(&kitchen[step_to_perform]);
			// Moving the chef to next state
			sem_wait(&state_mutex);

			int temp_cur = chef_current_state;
			
			chef_state[*chef_id - 1] = step_to_perform;
			if (temp_cur != IDLE) {
				sem_post(&kitchen[temp_cur]);
			}


			sem_post(&state_mutex);

			sem_wait(&next_state_mutex);
			chef_next_state[*chef_id - 1] = current_recipe->steps[current_recipe->next_action + 1].action;
			sem_post(&next_state_mutex);

			sem_wait(&priority_mutex);
			chef_priority[*chef_id - 1] = current_recipe->next_action;
			sem_post(&priority_mutex);

		}
	}

    else if (potential_deadlock == 1){
        
       
        if (chef_priority[*chef_id - 1] > chef_priority[waited_by]) {
            
            printf("NORMAL DEADLOCK: Chef %d waited too long for station %s, but it has higher priority, so it continues, released station %s\n", *chef_id, get_station_name(step_to_perform), get_station_name(chef_state[*chef_id - 1]));
            
            sem_post(&priority_mutex);
            
            sem_wait(&kitchen[step_to_perform]); // If the part of this kitchen is being used, sleep
            sem_wait(&state_mutex);
            
           int temp_cur = chef_current_state;
			chef_state[*chef_id - 1] = step_to_perform;
			if (temp_cur != IDLE) {
				sem_post(&kitchen[temp_cur]);
			}
            
            sem_post(&state_mutex);
            
            sem_wait(&next_state_mutex);
            chef_next_state[*chef_id - 1] = current_recipe->steps[current_recipe->next_action + 1].action;
            sem_post(&next_state_mutex);
            
            sem_wait(&priority_mutex);
            chef_priority[*chef_id - 1] = current_recipe->next_action;
            sem_post(&priority_mutex);
            
        } else{
            
            printf("XXXXXXXXXXXX  NORMAL DEADLOCK, Chef %d drop order %d, released station %s\n", *chef_id, order_number, get_station_name(chef_state[*chef_id - 1]));
            
            sem_post(&priority_mutex);
            sem_wait(&state_mutex);
            int temp_cur = chef_current_state;
			chef_state[*chef_id - 1] = IDLE;
			if (temp_cur != IDLE) {
				sem_post(&kitchen[temp_cur]);
			}
            sem_post(&state_mutex);
            
            sem_wait(&next_state_mutex);
            chef_next_state[*chef_id - 1] = IDLE;
            sem_post(&next_state_mutex);
            
            sem_wait(&lor_mutex);
            current_recipe->next_action = 0;
            current_recipe->is_done = 2;
            current_recipe->in_progress = 0;
            sem_post(&lor_mutex);
            
            
            sem_wait(&priority_mutex);
            chef_priority[*chef_id - 1] = 0;
            sem_post(&priority_mutex);
            
        }
        
    } else {

		sem_wait(&kitchen[step_to_perform]);
        sem_wait(&state_mutex);
        
        printf("Chef %d released station %s\n", *chef_id, get_station_name(chef_state[*chef_id - 1]));
        
        int temp_cur = chef_current_state;
			chef_state[*chef_id - 1] = step_to_perform;
			if (temp_cur != IDLE) {
				sem_post(&kitchen[temp_cur]);
			}
        
        sem_post(&state_mutex);
        
        sem_wait(&next_state_mutex);
        chef_next_state[*chef_id - 1] = current_recipe->steps[current_recipe->next_action + 1].action;
        sem_post(&next_state_mutex);
        
        sem_wait(&priority_mutex);
        chef_priority[*chef_id - 1] = current_recipe->next_action;
        sem_post(&priority_mutex);
        
        
    }
    sem_wait(&state_mutex);
    printf("Chef 1: %s, chef 2: %s, chef 3: %s\n", get_station_name(chef_state[0]), get_station_name(chef_state[1]), get_station_name(chef_state[2]));
    sem_post(&state_mutex);
}

/**
 * Function to leave the critical region
 */
void leave_station(int *chef_id, recipe *current_recipe, int order_number){
    
    sem_wait(&state_mutex);
    
    if (chef_state[*chef_id - 1] != -1){
        
        int step_finished = current_recipe->steps[current_recipe->next_action].action; // Get the step that the chef just finished
        
        //printf("Chef %d finished step %s of order %d, is leaving station %s\n", *chef_id, get_station_name(step_finished), order_number, get_station_name(step_finished));
        
        // Move to next step on the recipe steps, if no more step, mark the recipe as finished.
        if (current_recipe->next_action == current_recipe->num_action - 1) {
            current_recipe->is_done = 1;
            current_recipe->in_progress = 0;
				sem_post(&kitchen[chef_state[*chef_id - 1]]);
				chef_state[*chef_id - 1] = -1;
				sem_wait(&next_state_mutex);
				chef_next_state[*chef_id - 1] = -1;
				sem_post(&next_state_mutex);
            //printf("Order %d is finished by chef %d\n", order_number, *chef_id);
        } else {
            current_recipe->next_action++;
        }
    }
    
    sem_post(&state_mutex);
    
    // printf("Chef %d needs to leave station %s\n", *chef_id, get_station_name(step_finished));
    
    
}

/**
 * Function to perform the step
 * Takes in a recipe
 */
void perform_step (int *chef_id, recipe *current_recipe, int order_number) {
    
    sem_wait(&state_mutex);
    
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
    
    sem_post(&state_mutex);
    
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
            
            sem_wait(&lor_mutex);
            
            int k;
            printf("Chef %d checking for dropped order\n", *chef_id);
            for(k = 0; k < 30; k++){
                
                if(orders[k].is_done == 2){
                    order_num = k + 1;
                    current_recipe = &orders[k];
                    current_recipe->is_done = 0;
                    current_recipe->in_progress = 1;
                    printf("Chef %d found dropped order\n", *chef_id);
                    break;
                }
            }
            if(current_recipe == NULL){
                
                
                current_recipe = next_order(orders, &order_cursor, N);
                
                order_num = order_cursor;
            }
            sem_post(&lor_mutex);
            
            //printf("Chef %d got order %d, recipe %d\n", *chef_id, order_num, current_recipe->recipe_type);
            
        } else { /** Otherwise continue on the current order. */
            
            if (current_recipe->is_done == 2) {
                printf("Chef %d move onto next order\n", *chef_id);
                sem_wait(&lor_mutex);
                current_recipe->in_progress = 0;
                current_recipe = next_order(orders, &order_cursor, N);
                order_num = order_cursor;
                sem_post(&lor_mutex);
            }
            
            else if (current_recipe->is_done == 0) {
                
                printf("Chef %d is working on order %d, recipe %d\n", *chef_id, order_num, current_recipe->recipe_type);
                
                enter_station(chef_id, current_recipe, order_num);
                
                perform_step(chef_id, current_recipe, order_num);
                
                leave_station(chef_id, current_recipe, order_num);
                
                sem_wait(&priority_mutex);
                sem_wait(&lor_mutex);
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
                sem_post(&lor_mutex);
                sem_post(&priority_mutex);
                
            } else if (current_recipe->is_done == 1){
                
                printf(" ================ Chef %d finished order %d recipe %d ================\n", *chef_id, order_num, current_recipe->recipe_type);
                
                current_recipe = NULL;
                
            }
        }
        
    }
    
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

int check_deadlock(int value){
	int a = 0;
	for (a = 0; a < 3; a++) {
		if (chef_next_state[a] == value) return 1;
	}

	return 0;
}

int is_lowest_priority(int value){
	if (value == 0){
		if (chef_state[0] <= chef_state[1] && chef_state[0] <= chef_state[2]) return 1;
	}
	else if (value == 1){
		if (chef_state[1] <= chef_state[2] && chef_state[1] <= chef_state[0]) return 1;
	}
	else if (value == 2){
		if (chef_state[2] <= chef_state[1] && chef_state[2] <= chef_state[1]) return 1;
	}
	return 0;
}

