all: shell2

shell2: shell2.o linked_list.o
		gcc -g shell2.o linked_list.o -o shell2

shell2.o: shell2.c linked_list.h
		gcc -c -g shell2.c

linked_list.o: linked_list.c linked_list.h
		gcc -g -c linked_list.c

clean:
		rm -f *.o shell2
