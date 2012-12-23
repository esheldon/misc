CC=mpicc
#CFLAGS=-std=gnu99 -Wall -Werror
CFLAGS=-std=gnu99

SRC = minions.c

OBJ = minions.o

.c.o:
	$(CC) $(CFLAGS)  -c $<

all: $(OBJ)
	$(CC) $(OBJ) -o minions

clean:
	rm -f  core a.out *.o minions

