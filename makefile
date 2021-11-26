CFLAGS=-std=c99 -Wall -Wextra -Werror
pwcheck: setcal.o
	gcc -o $@ $^
pwcheck.o: setcal.c
	gcc $(CFLAGS) -c $^ 
clean: 
	rm -rd *.o pwcheck