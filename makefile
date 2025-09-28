CC=gcc
CFLAGS=-Wall -g

all: main

main: main.o funciones.o
	$(CC) $(CFLAGS) -o main main.o funciones.o

main.o: main.c funciones.h
	$(CC) $(CFLAGS) -c main.c

funciones.o: funciones.c funciones.h
	$(CC) $(CFLAGS) -c funciones.c

clean:
	rm -f *.o main

