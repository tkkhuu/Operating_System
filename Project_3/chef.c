#define N 30 // Number of orders
#define CHEF_NUM 3


#define PREP    0
#define STOVE   1
#define OVEN    2
#define SINK    3
#define IDLE    4

int orders[30];
int state[CHEF_NUM];

typedef int semaphore;

//semaphore mutex = 1;

semaphore kitchen[4] = { 1, 1, 1, 1 }; /* Control access over {Prep Area, Stove Top, Oven, Sink} */

/** ================= Data structures to represents a recipe. ================= */

/**
 * A struct that can represent a step of the kitchen
 */
typedef struct {
    
    /** An action in the kitchen: PREP, STOVE, OVEN, SINK. */
    int action;
    
    /** The time taken for this step in miliseconds. */
    int time_period;
    
} kitchen_step;


/**
 * A struct that can represent a recipe
 */
typedef struct {
    
    /** An array of kitchen_step to be performed. */
    kitchen_step steps[];
    
    /** A cursor to next action to be performed in the array. */
    unsigned short next_action;
    
    /** A boolean to determine whether this recipe is finished, 0 means unfinished and 1 means finished. */
    unsigned short is_done;
    
    /** The number of steps in this recipe, which is the size of steps[]. */
    unsigned short num_action;
    
} recipe;

/**
 * Function to generate a recipe.
 */
recipe generate_recipe(unsigned short rep_num){
    
    recipe my_recipe;
    
    my_recipe.is_done = 0;
    
    switch (rep_num) {
            
        case 1:
            
            my_recipe.next_action = 0;
            my_recipe.num_action = 6;
            my_recipe.steps = kitchen_step[num_action];
            
            my_recipe.steps[0].action = PREP;
            my_recipe.steps[0].time_period = 3000;
            
            my_recipe.steps[1].action = STOVE;
            my_recipe.steps[1].time_period = 4000;
            
            my_recipe.steps[2].action = SINK;
            my_recipe.steps[2].time_period = 2000;
            
            my_recipe.steps[3].action = PREP;
            my_recipe.steps[3].time_period = 2000;
            
            my_recipe.steps[4].action = OVEN;
            my_recipe.steps[4].time_period = 5000;
            
            my_recipe.steps[5].action = SINK;
            my_recipe.steps[5].time_period = 10000;
            
            break;
            
        case 2:
            
            my_recipe.next_action = 0;
            my_recipe.num_action = 3;
            my_recipe.steps = kitchen_step[num_action];
            
            my_recipe.steps[0].action = PREP;
            my_recipe.steps[0].time_period = 5000;
            
            my_recipe.steps[1].action = STOVE;
            my_recipe.steps[1].time_period = 3000;
            
            my_recipe.steps[2].action = SINK;
            my_recipe.steps[2].time_period = 15000;
            
            break;
            
        case 3:
            
            my_recipe.next_action = 0;
            my_recipe.num_action = 3;
            my_recipe.steps = kitchen_step[num_action];
            
            my_recipe.steps[0].action = PREP;
            my_recipe.steps[0].time_period = 10000;
            
            my_recipe.steps[1].action = OVEN;
            my_recipe.steps[1].time_period = 5000;
            
            my_recipe.steps[2].action = SINK;
            my_recipe.steps[2].time_period = 5000;
            
            break;
            
        case 4:
            
            my_recipe.next_action = 0;
            my_recipe.num_action = 3;
            my_recipe.steps = kitchen_step[num_action];
            
            my_recipe.steps[0].action = OVEN;
            my_recipe.steps[0].time_period = 15000;
            
            my_recipe.steps[1].action = PREP;
            my_recipe.steps[1].time_period = 5000;
            
            my_recipe.steps[2].action = SINK;
            my_recipe.steps[2].time_period = 4000;
            
            break;
            
        case 5:
            
            my_recipe.next_action = 0;
            my_recipe.num_action = 6;
            my_recipe.steps = kitchen_step[num_action];
            
            my_recipe.steps[0].action = PREP;
            my_recipe.steps[0].time_period = 2000;
            
            my_recipe.steps[1].action = OVEN;
            my_recipe.steps[1].time_period = 3000;
            
            my_recipe.steps[2].action = SINK;
            my_recipe.steps[2].time_period = 2000;
            
            my_recipe.steps[3].action = PREP;
            my_recipe.steps[3].time_period = 2000;
            
            my_recipe.steps[4].action = OVEN;
            my_recipe.steps[4].time_period = 3000;
            
            my_recipe.steps[5].action = SINK;
            my_recipe.steps[5].time_period = 4000;
            
            break;
    }
    
    return my_recipe;
}

/**
 * The main thread chef, takes in the number to identify that chef and a pointer to the recipe that the chef will work on.
 */
void chef(int i, recipe *current_recipe){
    
    while (TRUE) {
        if (current_recipe->is_done == 0) {
            enter_station(i, current_recipe);
            perform_step(current_recipe);
            leave_station(i, current_recipe);
        }
        
    }
    
}

/**
 * Function to enter the stations, which is the critical regions.
 */
void enter_station(int i, recipe *current_recipe){
    
    int step_to_perform = current_recipe->steps[current_recipe>next_action].action; // Next step to be performed: will either be PREP, STOVE, OVEN or SINK
    
    down(&kitchen[step_to_perform]); // If the part of this kitchen is being used, block
    
}


/**
 * Function to leave the critical region
 */
void leave_station(int i, recipe *current_recipe){
    
    int step_finished = current_recipe->steps[current_recipe->next_action].action; // Get the step that the chef just finished
    
    // Move to next step on the recipe steps, if no more step, mark the recipe as finished.
    if (current_recipe->next_action == current_recipe->num_action) {
        current_recipe->is_done = 1;
    } else {
        current_recipe->next_action++;
    }
    
    // Release the semaphore for the station.
    up(&kitchen[step_finished]);
}

/**
 * Function to perform the step
 * Takes in a recipe
 */
void perform_step (recipe *current_recipe) {
    
    sleep(current_recipe->steps[current_recipe->next_action].time_period);
    
}


/**
 void test(int i, recipe current_recipe){
 int l_counter;
 for (l_counter = 0; l_counter < CHEF_NUM; l_counter++) {
 if (state[l_counter] == current_recipe.steps[current_recipe.next_action].action) {
 return;
 }
 }
 
 state[i] = current_recipe.steps[current_recipe.next_action].action;
 sleep(current_recipe.steps[current_recipe.next_action].time_period);
 
 }
 */

/**
 * A situation when deadlock happens is when 3 chefs are waiting to enter each other's part of the kitchen, for example chef 1 in stove, chef 2 in oven and chef 3 in sink.
 * Chef 1 is waiting to enter oven, chef 2 is waiting to enter sink and chef 3 is waiting to enter stove.
 */


