#include "kitchen.h"

/** ================= Data structures to represents a recipe. ================= */

/**
 * Function to generate a recipe.
 */
recipe generate_recipe(unsigned int rep_num){

	recipe my_recipe;

	my_recipe.is_done = 0;

	my_recipe.recipe_type = rep_num;

	my_recipe.in_progress = 0;

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
recipe *next_order(recipe *orders, int *current_order, int order_size){

	recipe *next_order = &(orders[*current_order]);

	if (next_order->is_done == 1 || next_order->in_progress == 1) {
		next_order =  NULL;
	}

else {

		next_order->in_progress = 1;
	}
	(*current_order)++;

	if ((*current_order) >= order_size) {
		(*current_order) = 0;
	}

	return next_order;
}

/**
 * Funtion to get the name of a station.
 */
char *get_station_name(int station_id){

	switch (station_id) {
		case PREP:
			return "Prep Area";
			break;

		case STOVE:
			return "Stove Top";
			break;

		case OVEN:
			return "Oven";
			break;

		case SINK:
			return "Sink";
			break;

		default:
			return "Idle";
			break;
	}
}


