SHELL      = /bin/sh

CPP=g++
CC=g++
LINK=g++

OPTS= -O3 -funroll-loops
#OPTS= -g

LIBRARIES=-lm

EXECNAME="e4.x"

# objects to compile
OBJ = calcMagErr4.o hpsel.o sort.o

# General rule for compiling
all: usual

clean: 
	rm *.o $(EXECNAME)

usual: $(OBJ)
	$(LINK) $(OPTS) -o $(EXECNAME) $(OBJ) $(LIBRARIES)

#list of rules for each individual file
calcMagErr4.o: calcMagErr4.cpp
	$(CPP) $(OPTS) -c -o calcMagErr4.o calcMagErr4.cpp

hpsel.o: hpsel.cpp
	$(CPP) $(OPTS) -c -o hpsel.o hpsel.cpp

sort.o: sort.cpp
	$(CPP) $(OPTS) -c -o sort.o sort.cpp
