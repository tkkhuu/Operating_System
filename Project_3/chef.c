#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/time.h>
#include <time.h>

#define N 30 // Number of orders
#define CHEF_NUM 3
#define TRUE 1


#define PREP    0
#define STOVE   1
#define OVEN    2
#define SINK    3
#define IDLE    4

/** ================= Data structures to represents a recipe. ================= */

/**
 * A struct that can represent a step of the kitchen
 */
struct kitchen_step_struct {

	/** An action in the kitchen: PREP, STOVE, OVEN, SINK. */
	int action;

	/** The time taken for this step in seconds. */
	int time_period;

};

typedef struct kitchen_step_struct kitchen_step;

/**
 * A struct that can represent a recipe
 */
struct recipe_struct {

	/** Recipe type. */
	int recipe_type;

	/** An array of kitchen_step to be performed. */
	kitchen_step *steps;

	/** A cursor to next action to be performed in the array. */
	unsigned int next_action;

	/** A boolean to determine whether this recipe is finished, 0 means unfinished and 1 means finished. */
	unsigned short is_done;

	/** The number of steps in this recipe, which is the size of steps[]. */
	unsigned int num_action;

};

typedef struct recipe_struct recipe;

void chef(int *chef_id);
void enter_station(int *chef_id, recipe *current_recipe);
void leave_station(int *chef_id, recipe *current_recipe);
void perform_step (int *chef_id, recipe *current_recipe);

int current_order = 0;

sem_t kitchen[4];

sem_t recipe_mutex[5];



sem_t lor_mutex;


recipe orders[N];

/**
 * Function to generate a recipe.
 */
recipe generate_recipe(unsigned int rep_num){

	recipe my_recipe;

	my_recipe.is_done = 0;

	my_recipe.recipe_type = rep_num;

	switch (rep_num) {

		case 1:

			my_recipe.next_action = 0;
			my_recipe.num_action = 6;
			my_recipe.steps = malloc(my_recipe.num_action * sizeof(*my_recipe.steps));

			my_recipe.steps[0].action = PREP;
			my_recipe.steps[0].time_period = 3;

			my_recipe.steps[1].action = STOVE;
			my_recipe.steps[1].time_period = 4;

			my_recipe.steps[2].action = SINK;
			my_recipe.steps[2].time_period = 2;

			my_recipe.steps[3].action = PREP;
			my_recipe.steps[3].time_period = 2;

			my_recipe.steps[4].action = OVEN;
			my_recipe.steps[4].time_period = 5;

			my_recipe.steps[5].action = SINK;
			my_recipe.steps[5].time_period = 10;

			break;

		case 2:

			my_recipe.next_action = 0;
			my_recipe.num_action = 3;
			my_recipe.steps = malloc(my_recipe.num_action * sizeof(*my_recipe.steps));

			my_recipe.steps[0].action = PREP;
			my_recipe.steps[0].time_period = 5;

			my_recipe.steps[1].action = STOVE;
			my_recipe.steps[1].time_period = 3;

			my_recipe.steps[2].action = SINK;
			my_recipe.steps[2].time_period = 15;

			break;

		case 3:

			my_recipe.next_action = 0;
			my_recipe.num_action = 3;
			my_recipe.steps = malloc(my_recipe.num_action * sizeof(*my_recipe.steps));

			my_recipe.steps[0].action = PREP;
			my_recipe.steps[0].time_period = 10;

			my_recipe.steps[1].action = OVEN;
			my_recipe.steps[1].time_period = 5;

			my_recipe.steps[2].action = SINK;
			my_recipe.steps[2].time_period = 5;

			break;

		case 4:

			my_recipe.next_action = 0;
			my_recipe.num_action = 3;
			my_recipe.steps = malloc(my_recipe.num_action * sizeof(*my_recipe.steps));

			my_recipe.steps[0].action = OVEN;
			my_recipe.steps[0].time_period = 15;

			my_recipe.steps[1].action = PREP;
			my_recipe.steps[1].time_period = 5;

			my_recipe.steps[2].action = SINK;
			my_recipe.steps[2].time_period = 4;

			break;

		case 5:

			my_recipe.next_action = 0;
			my_recipe.num_action = 6;
			my_recipe.steps = malloc(my_recipe.num_action * sizeof(*my_recipe.steps));

			my_recipe.steps[0].action = PREP;
			my_recipe.steps[0].time_period = 2;

			my_recipe.steps[1].action = OVEN;
			my_recipe.steps[1].time_period = 3;

			my_recipe.steps[2].action = SINK;
			my_recipe.steps[2].time_period = 2;

			my_recipe.steps[3].action = PREP;
			my_recipe.steps[3].time_period = 2;

			my_recipe.steps[4].action = OVEN;
			my_recipe.steps[4].time_period = 3;

			my_recipe.steps[5].action = SINK;
			my_recipe.steps[5].time_period = 4;

			break;
	}

	return my_recipe;
}

/**
 * Function to get the next order
 */
recipe *next_order(){

	recipe *next_order = &(orders[current_order]);

	if (next_order->is_done == 1) {
		next_order = NULL;
	} 

	current_order++;

	if (current_order > N) {
		current_order = 0;
	}

	return next_order;

}

recipe oders[N];

/** ============================= Chef thread. ============================= */

/** ============================= Chef Actions ============================= */
/**
 * Function to enter the stations, which is the critical regions.
 */
void enter_station(int *chef_id, recipe *current_recipe){

	int step_to_perform = current_recipe->steps[current_recipe->next_action].action; // Next step to be performed: will either be PREP, STOVE, OVEN or SINK

	printf("Chef %d is waiting to enter station %d\n", *chef_id, step_to_perform);	
	
	sem_wait(&kitchen[step_to_perform]); // If the part of this kitchen is being used, sleep

}

/**
 * Function to leave the critical region
 */
void leave_station(int *chef_id, recipe *current_recipe){

	int step_finished = current_recipe->steps[current_recipe->next_action].action; // Get the step that the chef just finished

	printf("Chef %d is leaving station %d\n", *chef_id, step_finished);

	// Move to next step on the recipe steps, if no more step, mark the recipe as finished.
	if (current_recipe->next_action == current_recipe->num_action) {
		current_recipe->is_done = 1;
	} else {
		current_recipe->next_action++;
	}

	// Release the semaphore for the station.
	sem_post(&kitchen[step_finished]);

	printf("Chef %d left station %d\n", *chef_id, step_finished);
}

/**
 * Function to perform the step
 * Takes in a recipe
 */
void perform_step (int *chef_id, recipe *current_recipe) {

	int time_taken = current_recipe->steps[current_recipe->next_action].time_period;

	struct timeval start, end;

	int time_elapsed = 0;

	gettimeofday(&start, NULL);

	printf("Chef %d is working in station %d\n", *chef_id, current_recipe->steps[current_recipe->next_action].action);

	while ( time_elapsed < time_taken) {

		gettimeofday(&end, NULL);

		time_elapsed = (int)(end.tv_sec - start.tv_sec);

	}

}

char *get_station_name(int station_id){
	
}

/**
 * The main thread chef, takes in the number to identify that chef and a pointer to the recipe that the chef will work on.
 */
void chef(int *chef_id){

	recipe *current_recipe = NULL;

	while (TRUE) {

		/** If the chef is not working on any order, request an order. */
		if(current_recipe == NULL){

			printf("Chef %d is getting a recipe\n", *chef_id);

			sem_wait(&lor_mutex);

			current_recipe = next_order();

			sem_post(&lor_mutex);

			printf("Chef %d got recipe %d\n", *chef_id, current_recipe->recipe_type);

		} else { /** Otherwise continue on the current order. */

			if (current_recipe->is_done == 0) {

				printf("Chef %d is working on recipe %d\n", *chef_id, current_recipe->recipe_type);

				sem_wait(&recipe_mutex[current_recipe->recipe_type - 1]);

				enter_station(chef_id, current_recipe);

				perform_step(chef_id, current_recipe);

				leave_station(chef_id, current_recipe);

			} else if (current_recipe->is_done == 1){

				sem_post(&recipe_mutex[current_recipe->recipe_type - 1]);

				printf("Chef %d is finished recipe %d\n", *chef_id, current_recipe->recipe_type);

				current_recipe = NULL;

			}
		}

	}

}



/** ============================= Main function ============================= */

int main (int argc, char* argv[]){

	printf("Initializing semaphores ...\n");
	
	/** Define 3 threads for 3 chefs. */
	pthread_t chef_1, chef_2, chef_3;

	int chef_id_1 = 1; int chef_id_2 = 2; int chef_id_3 = 3;

	printf("Initializing orders semaphore ...\n");

	sem_init(&lor_mutex, 0, 1);

	printf("Initializing kitchen semaphores ...\n");

	sem_init(&kitchen[0], 0, 1);
	sem_init(&kitchen[1], 0, 1);
	sem_init(&kitchen[2], 0, 1);
	sem_init(&kitchen[3], 0, 1);

	printf("Initializing recipe semaphores ...\n");

	sem_init(&recipe_mutex[0], 0, 1);
	sem_init(&recipe_mutex[1], 0, 1);
	sem_init(&recipe_mutex[2], 0, 1);
	sem_init(&recipe_mutex[3], 0, 1);
	sem_init(&recipe_mutex[4], 0, 1);
	
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

/**
 * A situation when deadlock happens is when 3 chefs are waiting to enter each other's part of the kitchen, for example chef 1 in stove, chef 2 in oven and chef 3 in sink.
 * Chef 1 is waiting to enter oven, chef 2 is waiting to enter sink and chef 3 is waiting to enter stove.
 */


