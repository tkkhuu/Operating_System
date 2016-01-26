all: runCommand shell shell2

runCommand: runCommand.o
		gcc -g runCommand.o -o runCommand

runCommand.o: runCommand.c
		gcc -c -g runCommand.c

shell: shell.o
		gcc -g shell.o -o shell

shell.o: shell.c
		gcc -c -g shell.c

shell2: shell2.o linked_list.o
		gcc -g shell2.o linked_list.o -o shell2

shell2.o: shell2.c linked_list.h
		gcc -c -g shell2.c

linked_list.o: linked_list.c linked_list.h
		gcc -g -c linked_list.c

clean:
		rm -f *.o runCommand shell shell2
