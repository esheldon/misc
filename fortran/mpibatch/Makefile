###Carver
F90=mpif90
OFLAGS = --free-form
#
SRC = mpibatch.f 

OBJ = mpibatch.o

# generic compilation 
.f.o:
	$(F90) $(OFLAGS)  -c $<

all: $(OBJ)
	$(F90) $(OBJ) $(OFLAGS) -o mpibatch

clean:
	rm -f  core a.out *.o mpibatch

