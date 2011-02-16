SHELL      = /bin/sh

CPP=g++
LINK=g++

OPTS= -O2
#OPTS= -O2 -march=k8

LIBRARIES=-lm

EXECNAME="neuz.x"
FIT_EXEC="neu_fit.x"

# objects to compile
OBJECTS = main.o neu_net.o nr.o
FIT_OBJ = neu_net.o nr.o neu_fit.o

# General rules for making .o files
.C.o:
	$(CPP) -c $(POTS) $(OPTS) $(FFTWOPTS) $<

.cpp.o:
	$(CPP) $(POTS) -c $(OPTS) $(FFTWOPTS) $<

.f.o:
	g77 -c $<

# General rule for compiling
all: neu_net neu_fit

neu_net: $(OBJECTS)
	$(LINK) $(OPTS) -o $(EXECNAME) $(OBJECTS) $(LIBRARIES)

neu_fit: $(FIT_OBJ)
	$(LINK) $(OPTS) -o $(FIT_EXEC) $(FIT_OBJ) $(LIBRARIES)

clean: 
	rm *.o $(EXECNAME) $(FIT_EXEC)

