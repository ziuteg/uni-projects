CC = gcc
CFLAGS = -std=gnu99 -Wall -Wextra

router: main.o router.o distance_vector.o utilities.o
main.o: main.c utilities.h
router.o: router.c router.h distance_vector.h utilities.h
distance_vector.o: distance_vector.c distance_vector.h utilities.h
utilities.o: utilities.c utilities.h

clean:
	rm -f *.o
distclean:
	rm -f *.o *~ router
