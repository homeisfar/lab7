#
#   Makefile for building the cache simulator
#
#CFLAGS = -O1 -Wall
CFLAGS = -O0 -ggdb3 -Wall


all:   pdp429

pdp429: pdp8.o objinput.o
	gcc $(CFLAGS) pdp8.o objinput.o -o pdp429

pdp8.o: pdp8.c objinput.h
	gcc $(CFLAGS) pdp8.c -c

clean: 
	rm -f *.o pdp429

