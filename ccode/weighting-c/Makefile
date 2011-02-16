SHELL      = /bin/sh

CC=c99

OPTS= -O3 -funroll-loops -g

#LIBRARIES=-lm
LIBRARIES=-lgsl -lgslcblas -lm

OBJS=Point.o HCube.o
HEADERS=Point.h HCube.h

EXEC=test
default: $(EXEC)
clean:
	rm -f $(EXEC) *.o


test: test.c $(OBJS) $(HEADERS)
	$(CC) $(OPTS) -o $@ $(OBJS) $(LIBRARIES) $@.c

Point.o: Point.c Point.h
	$(CC) $(OPTS) -c Point.c

HCube.o: HCube.c HCube.h
	$(CC) $(OPTS) -c HCube.c
