
CC =		g++
LD = 		g++

HTMDIR	=	../htm
INCLUDE = -I${HTMDIR}
CPPFLAGS =	-D_FILE_OFFSET_BITS=64 -Wno-deprecated -O3 ${INCLUDE} 
CP      =       cp
RM      =       rm -f

OBJLIST = correlate.o corrobj.o angDist.o gcirc.o kcorr.o binner.o
EXECUTABLES = correlate

LIBRARY = ${HTMDIR}/libSpatialIndex.a

LDLIBS  = ${LIBRARY} -lm


# rules
all: ${EXECUTABLES}
depend:
	makedepend -- -Y $(CFLAGS) -- *.cpp

clean:
	- ${RM} *.o ${EXECUTABLES}


# Executables
correlate: ${OBJLIST}
	       @echo linking $@
	       - ${LD} -o $@ ${LDFLAGS} ${OBJLIST} ${LDLIBS}
.cpp.o:
	    ${CC} -c ${CPPFLAGS} $<


# DO NOT DELETE

angDist.o: angDist.h types.h Array.h
binner.o: binner.h types.h Array.h
correlate.o: corrobj.h Array.h types.h constants.h kcorr.h binner.h gcirc.h
correlate.o: angDist.h
corrobj.o: corrobj.h Array.h types.h constants.h kcorr.h binner.h gcirc.h
corrobj.o: angDist.h
gcirc.o: gcirc.h types.h Array.h constants.h
kcorr.o: kcorr.h Array.h types.h constants.h
