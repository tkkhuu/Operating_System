#define N 30 // Number of orders
#define CHEF_NUM 3

#define IDLE    0
#define PREP    1
#define STOVE   2
#define TOP     3
#define OVEN    4
#define SINK    5

int orders[30];
int state[CHEF_NUM];

typedef int semaphore;
semaphore mutex = 1;
semaphore s[4] = { 1, 1, 1, 1 }; /* Control access over {Prep Area, Stove Top, Oven, Sink} */

/** ================= Data structures to represents a recipe. ================= */
typedef struct {
    
    int action;
    int time_period;
    
} kitchen_step;

typedef struct {
    
    kitchen_step steps[];
    
    int next_action;
    
} recipe;

/**
 * Function to generate a recipe.
 */
recipe generate_recipe(int rep_num){
    
    recipe my_recipe;
    
    switch (rep_num) {
            
        case 1:
            
            my_recipe.next_action = 0;
            my_recipe.steps = kitchen_step[6];
            
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
            my_recipe.steps = kitchen_step[3];
            
            my_recipe.steps[0].action = PREP;
            my_recipe.steps[0].time_period = 5000;
            
            my_recipe.steps[1].action = STOVE;
            my_recipe.steps[1].time_period = 3000;
            
            my_recipe.steps[2].action = SINK;
            my_recipe.steps[2].time_period = 15000;
            
            break;
            
        case 3:
            
            my_recipe.next_action = 0;
            my_recipe.steps = kitchen_step[3];
            
            my_recipe.steps[0].action = PREP;
            my_recipe.steps[0].time_period = 10000;
            
            my_recipe.steps[1].action = OVEN;
            my_recipe.steps[1].time_period = 5000;
            
            my_recipe.steps[2].action = SINK;
            my_recipe.steps[2].time_period = 5000;
            
            break;
            
        case 4:
            
            my_recipe.next_action = 0;
            my_recipe.steps = kitchen_step[3];
            
            my_recipe.steps[0].action = OVEN;
            my_recipe.steps[0].time_period = 15000;
            
            my_recipe.steps[1].action = PREP;
            my_recipe.steps[1].time_period = 5000;
            
            my_recipe.steps[2].action = SINK;
            my_recipe.steps[2].time_period = 4000;

            break;
            
        case 5:
            
            my_recipe.next_action = 0;
            my_recipe.steps = kitchen_step[6];
            
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

void chef(int i, recipe current_recipe){
    
    while (TRUE) {
        //
    }
    
}

void perform_step(int i, recipe current_recipe){
    down(&mutex);
    test(i, current_recipe);
    up(&mutex);
}

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