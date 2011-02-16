SHELL      = /bin/sh

# this will get the command line if it exists, otherwise take this default
PREFIX:=~/local

CC=g++
LINK=g++

OPTS= -O3 -funroll-loops -g

LIBRARIES=-lm


EXEC= calcweights calcpofz
SCRIPT=calcpofz.sh

# objects to compile
OBJS = weights.o util.o KDTree.o whist.o 
HEADERS= Catalog.h  Hcube.h  KDTree.h  Point.h  util.h  weights.h PofZ.h whist.h

UPS_TABLE= ups/weighting.table

default: $(EXEC)

all: default

clean: 
	rm -f $(OBJS) $(EXEC)

install: $(EXEC) $(SCRIPT)
	mkdir -p $(PREFIX)
	mkdir -p $(PREFIX)/bin
	mkdir -p $(PREFIX)/ups

	cp -fv $(EXEC) $(PREFIX)/bin/
	cp -fv $(SCRIPT) $(PREFIX)/bin/
	cp -fv $(UPS_TABLE) $(PREFIX)/ups/

calcweights: $(OBJS) calcweights.cc
	$(LINK) $(OPTS) -o $@ $(OBJS) $(LIBRARIES) $@.cc
calcpofz: $(OBJS) calcpofz.cc
	$(LINK) $(OPTS) -o $@ $(OBJS) $(LIBRARIES) $@.cc

test: $(OBJS) test.cc
	$(LINK) $(OPTS) -o $@ $(OBJS) $(LIBRARIES) $@.cc

weights.o: weights.cc $(HEADERS)
	$(CC) $(OPTS) -c weights.cc

util.o: util.cc $(HEADERS)
	$(CC) $(OPTS) -c util.cc

whist.o: whist.cc $(HEADERS)
	$(CC) $(OPTS) -c whist.cc

KDTree.o: KDTree.cc $(HEADERS)
	$(CC) $(OPTS) -c KDTree.cc
