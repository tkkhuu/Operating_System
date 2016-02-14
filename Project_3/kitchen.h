#ifndef _CHEF_H

#include <stdio.h>
#include <stdlib.h>


#define PREP   0
#define STOVE  1
#define OVEN   2
#define SINK   3
#define IDLE  -1



/**
 * A struct that can represent a step of the kitchen
 */
typedef struct kitchen_step_struct {
    
    /** An action in the kitchen: PREP, STOVE, OVEN, SINK. */
    int action;
    
    /** The time taken for this step in seconds. */
    int time_period;
    
} kitchen_step;

/**
 * A struct that can represent a recipe
 */
typedef struct recipe_struct {
    
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
    
} recipe;

/** ============ Function prototypes ============ */

char *get_station_name(int station_id);

recipe generate_recipe(unsigned int rep_num);

recipe *next_order(recipe *orders, int *current_order, int order_size);

char *get_station_name(int station_id);

#endif
