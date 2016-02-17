
The project is to design a synchronized system for 3 chefs to work in the kitchen.

We populate an array of 30 orders from 5 different recipes, an alternative way is to have a thread that generate orders at random times. But this will not effect how the chef works.

We are able to satisfy the conditions:
- No 2 chefs can be in the same station
- Chef can’t go to wait room until his order is finished or dropped
- Maximize parallelism
- A dropped order will have the priority to be picked up first so it doesn’t starve
- Deadlock it prevented

Since we did not have a thread that generate order, the orders toward the end of the array will expect to wait longer. Therefore in the real scenario, when there are some time limit between orders, each orders will expect to wait for shorter amount of time. So we our program is actually running in the worst case scenario where orders come right after another without any time in between.