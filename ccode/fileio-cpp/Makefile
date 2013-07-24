CC =	g++
LD =	g++

CCFLAGS = -I${IDL_DIR}/external -fPIC -O -Wno-deprecated -D_FILE_OFFSET_BITS=64
LDFLAGS = -shared
CP =	cp -f
RM =	rm -f


SOFILES =	binary_readIDL.so ascii_readIDL.so ascii_writeIDL.so

all: ${SOFILES}

binary_readIDL.so: binary_readIDL.o FileObj.o
	@ echo linking $@
	${LD} ${LDFLAGS} -o $@ binary_readIDL.o FileObj.o

ascii_readIDL.so: ascii_readIDL.o FileObj.o
	@ echo linking $@
	${LD} ${LDFLAGS} -o $@ ascii_readIDL.o FileObj.o

ascii_writeIDL.so: ascii_writeIDL.o FileObj.o
	@ echo linking $@
	${LD} ${LDFLAGS} -o $@ ascii_writeIDL.o FileObj.o



clean:
	- ${RM} *.o *.so


.SUFFIXES:
.SUFFIXES: .o .cpp

.cpp.o:
	@ echo compiling $<
	${CC} -c ${CCFLAGS} $<


binary_readIDL.o: binary_readIDL.cpp types.hpp keywords.hpp
ascii_readIDL.o: ascii_readIDL.cpp types.hpp keywords.hpp
ascii_writeIDL.o: ascii_writeIDL.cpp types.hpp keywords.hpp
FileObj.o: FileObj.cpp FileObj.hpp types.hpp keywords.hpp
