all: chef_sem chef_cond

chef_sem: chef_sem.o kitchen.o
	gcc -pthread -g chef_sem.o kitchen.o -o chef_sem

chef_cond: chef_cond.o kitchen.o
	gcc -pthread -g chef_cond.o kitchen.o -o chef_cond

chef_sem.o: chef_sem.c kitchen.h
	gcc -c -g chef_sem.c

chef_cond.o: chef_cond.c kitchen.h
	gcc -c -g chef_cond.c

kitchen.o: kitchen.c kitchen.h
	gcc -g -c kitchen.c

clean:
	rm -f *.o chef_sem chef_cond
