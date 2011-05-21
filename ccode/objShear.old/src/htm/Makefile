#	PACKAGE:	SpatialIndex
#
# os: linux
# --- Macros -----------------------------------------------------------------


CCXX = 		g++
LD = 		g++

INCLUDE =	-I./
CCFLAGS =	${INCLUDE} -O3 -fPIC -Wno-deprecated
ARFLAGS = 	-rlcs
CP      =       cp
RM      =       rm -f
LIBRARY =	libSpatialIndex.a

#LIBOBJS =       BitList.o\
                SpatialVector.o\
                SpatialIndex.o\
                SpatialInterface.o\
                SpatialEdge.o\
                SpatialException.o\
                SpatialConstraint.o\
                SpatialConvex.o\
                SpatialDomain.o

LIBOBJS = 	BitList.o \
		  	SpatialConstraint.o \
			SpatialConvex.o \
			SpatialDomain.o \
			SpatialEdge.o \
			SpatialException.o \
			SpatialIndex.o \
			SpatialInterface.o \
			SpatialVector.o \
			VarStr.o


# --- Targets ----------------------------------------------------------------

# Build and install the library.

all : ${LIBRARY}
lib : ${LIBRARY}

${LIBRARY} : ${LIBOBJS}
	${AR} ${ARFLAGS} $@ $?

lookup: lookup.o ${LIBRARY}
	${LD} -o lookup ${CCFLAGS} lookup.o ${LIBRARY}

clean :	
	- ${RM} ${LIBRARY}
	- ${RM} ${LIBOBJS}

# --- Suffix Rules -----------------------------------------------------------

# First clear out the default suffixes, then declare our own and
# define the rules.

.SUFFIXES:
.SUFFIXES: .o .cpp

# Compiling C++ code.  The command lines are so long as to be useless.
# Use 'sxmake -terse' to suppress the echo of the compile command.

.cpp.o:
	${CCXX} -c ${CCFLAGS} $<

