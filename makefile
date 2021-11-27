CFLAGS=-std=c99 -Wall -Wextra -Werror
setcal: setcal.o
	gcc -o $@ $^
setcal.o: setcal.c
	gcc $(CFLAGS) -g -c $^ 
clean: 
	rm -rd *.o setcal