SHELL      = /bin/sh

CC=c99
LD=c99

#OPTS= -O3 -funroll-loops -g
#OPTS= -O3 -g
OPTS= -O3 -g

# if NDIM is not sent with "make NDIM=" then default is 5
NDIM:=5
# this will get the command line if it exists, otherwise take this default
PREFIX:=~/local

CCFLAGS = -DNDIM$(NDIM)

LIBRARIES=-lm

OBJS=util.o partition.o Points.o HCube.o TreeNode.o KDTree.o heap.o \
	PhotoCatalog.o TrainCatalog.o WHist.o Vector.o
HEADERS=util.h partition.h Points.h HCube.h TreeNode.h KDTree.h heap.h \
	PhotoCatalog.h TrainCatalog.h WHist.h Vector.h params.h dims.h

UPS_TABLE= weighting.table

CALCWEIGHTS_EXEC=calcweights$(NDIM)
CALCPOFZ_EXEC=calcpofz$(NDIM)

TESTEXEC=test$(NDIM)
EXEC=$(CALCWEIGHTS_EXEC) $(CALCPOFZ_EXEC)

default: $(EXEC)
test: $(TESTEXEC)
clean:
	rm -f $(TESTEXEC) $(EXEC) *.o


install: $(EXEC)
	mkdir -p $(PREFIX)
	mkdir -p $(PREFIX)/bin
	mkdir -p $(PREFIX)/ups

	cp -f $(EXEC) $(PREFIX)/bin/
	cp -f ups/$(UPS_TABLE) $(PREFIX)/ups/

uninstall:
	rm -f $(PREFIX)/bin/$(CALCWEIGHTS_EXEC)
	rm -f $(PREFIX)/bin/$(CALCPOFZ_EXEC)
	rm -f $(PREFIX)/ups/$(UPS_TABLE)


$(TESTEXEC): tester.c $(OBJS) $(HEADERS)
	@echo making $(TESTEXEC)
	$(LD) $(OPTS) $(CCFLAGS) -o $@ $(OBJS) $(LIBRARIES) tester.c


$(CALCWEIGHTS_EXEC): calcweights.c $(OBJS) $(HEADERS)
	@echo making $(CALCWEIGHTS_EXEC)
	$(LD) $(OPTS) $(CCFLAGS) -o $@ $(OBJS) $(LIBRARIES) calcweights.c

$(CALCPOFZ_EXEC): calcpofz.c $(OBJS) $(HEADERS)
	@echo making $(CALCPOFZ_EXEC)
	$(LD) $(OPTS) $(CCFLAGS) -o $@ $(OBJS) $(LIBRARIES) calcpofz.c

heap.o: heap.c heap.h
	$(CC) $(OPTS) $(CCFLAGS) -c heap.c

PhotoCatalog.o: PhotoCatalog.c PhotoCatalog.h params.h dims.h
	$(CC) $(OPTS) $(CCFLAGS) -c PhotoCatalog.c

TrainCatalog.o: TrainCatalog.c TrainCatalog.h params.h dims.h
	$(CC) $(OPTS) $(CCFLAGS) -c TrainCatalog.c

Points.o: Points.c Points.h params.h dims.h
	$(CC) $(OPTS) $(CCFLAGS) -c Points.c

HCube.o: HCube.c HCube.h params.h dims.h
	$(CC) $(OPTS) $(CCFLAGS) -c HCube.c

TreeNode.o: TreeNode.c TreeNode.h params.h dims.h
	$(CC) $(OPTS) $(CCFLAGS) -c TreeNode.c

KDTree.o: KDTree.c KDTree.h params.h dims.h
	$(CC) $(OPTS) $(CCFLAGS) -c KDTree.c

partition.o: partition.c partition.h params.h dims.h
	$(CC) $(OPTS) $(CCFLAGS) -c partition.c

util.o: util.c util.h params.h dims.h
	$(CC) $(OPTS) $(CCFLAGS) -c util.c

Vector.o: Vector.c Vector.h params.h dims.h
	$(CC) $(OPTS) $(CCFLAGS) -c Vector.c

WHist.o: WHist.c WHist.h params.h dims.h
	$(CC) $(OPTS) $(CCFLAGS) -c WHist.c
