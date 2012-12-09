###Carver
CC=mpicc
#CFLAGS=-std=gnu99
CFLAGS=-std=gnu99 -Wall -Werror

SRC = mpibatch.c

OBJ = mpibatch.o

# generic compilation 
.c.o:
	$(CC) $(CFLAGS)  -c $<

all: $(OBJ)
	$(CC) $(OBJ) -o mpibatch

clean:
	rm -f  core a.out *.o mpibatch

