all: runChef

chef: chef.o kitchen.o
    gcc -g chef.o kitchen.o -o runChef

chef.o: chef.c kitchen.h
    gcc -c -g chef.c

kitchen.o: kitchen.c kitchen.h
    gcc -g -c kitchen.c

clean:
	rm -f *.o runChef
