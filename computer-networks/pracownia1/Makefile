CC = gcc
CFLAGS = -std=gnu99 -g -O2 -Wall -Wextra

traceroute: main.o traceroute.o utilities.o

main.o: main.c
traceroute.o: traceroute.c traceroute.h utilities.h
utilities.o: utilities.c utilities.h

clean:
	rm -f *~ *.o traceroute
