CFLAGS = -Wall -pedantic -g


dine: dine.o
	gcc $(CFLAGS) -o dine dine.o -pthread -lrt

dine.o: dine.c
	gcc $(CFLAGS) -c dine.c dine.h



clean:
	rm *~
