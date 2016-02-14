all: chef

chef: chef.o kitchen.o
	gcc -pthread -g chef.o kitchen.o -o chef

chef.o: chef.c kitchen.h
	gcc -c -g chef.c

kitchen.o: kitchen.c kitchen.h
	gcc -g -c kitchen.c

clean:
	rm -f *.o run_chef
