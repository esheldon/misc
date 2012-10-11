CC	= cc
LD	= cc
CCFLAGS	= -DLINKAGE -fPIC
LIBOBJS	= admom.o


default: all
all: ${LIBOBJS}

clean: 
	- rm -f ${LIBOBJS};

.SUFFIXES:
.SUFFIXES: .o .c

.c.o:
	@ echo compiling $<
	@ ${CC} -c ${CCFLAGS} $<
