CC=gcc
CFLAGS=-Wall -g

all: main

main: main.o
	$(CC) $(CFLAGS) -o main main.o

main.o: main.c funciones.h
	$(CC) $(CFLAGS) -c main.c

clean:
	rm -f *.o main

