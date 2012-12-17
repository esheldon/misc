CC=mpicc
#CFLAGS=-std=gnu99 -Wall -Werror
CFLAGS=-std=gnu99

SRC = mpibatch.c

OBJ = mpibatch.o

.c.o:
	$(CC) $(CFLAGS)  -c $<

all: $(OBJ)
	$(CC) $(OBJ) -o mpibatch

clean:
	rm -f  core a.out *.o mpibatch

