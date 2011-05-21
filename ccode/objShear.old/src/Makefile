CC = g++
LD = g++

TESTDIR		=	./test
HTMDIR		=	./htm
INCLUDE 	= 	-I${HTMDIR}
CPPFLAGS	=	-O ${INCLUDE} -Wno-deprecated
CP			= 	cp
RM			=	rm -f

OBJLIST 	=	LensSource.o sigmaCritInv.o angDist.o gcirc.o
EXECUTABLES	=	objshear
TESTEXEC	= 	$(TESTDIR)/test-lcat \
				$(TESTDIR)/test-scat \
				$(TESTDIR)/test-rev \
				$(TESTDIR)/test-gcirc \
				$(TESTDIR)/test-angdist \
				$(TESTDIR)/test-cosmo \
				$(TESTDIR)/test-scinv

LIBRARY		=	${HTMDIR}/libSpatialIndex.a

LDLIBS		=	${LIBRARY} -lm


# rules
all: ${EXECUTABLES}

clean:
	- ${RM} *.o test/*.o ${EXECUTABLES} $(TESTEXEC)


# Executables
objshear: objshear.o ${OBJLIST}
	       @echo linking $@
	       - ${LD} -o $@ ${LDFLAGS} objshear.o ${OBJLIST} ${LDLIBS}


test: $(TESTEXEC)

$(TESTDIR)/test-gcirc: $(TESTDIR)/test-gcirc.o gcirc.o
	- $(CC) -o $(TESTDIR)/test-gcirc $(TESTDIR)/test-gcirc.cpp gcirc.o -lm
$(TESTDIR)/test-scat: $(TESTDIR)/test-scat.o scat.o angDist.o
	- $(CC) -o $(TESTDIR)/test-scat $(TESTDIR)/test-scat.cpp scat.o angDist.o -lm

$(TESTDIR)/test-rev: $(TESTDIR)/test-rev.o rev.o
	- $(CC) -o $(TESTDIR)/test-rev $(TESTDIR)/test-rev.cpp rev.o

$(TESTDIR)/test-lcat: $(TESTDIR)/test-lcat.o lcat.o angDist.o lcat.h
	- $(CC) -o $(TESTDIR)/test-lcat $(TESTDIR)/test-lcat.cpp lcat.o angDist.o -lm
$(TESTDIR)/test-angdist: $(TESTDIR)/test-angdist.o angDist.o
	- $(CC) -o $(TESTDIR)/test-angdist $(TESTDIR)/test-angdist.cpp angDist.o -lm
$(TESTDIR)/test-cosmo: $(TESTDIR)/test-cosmo.o Cosmology.h
	- $(CC) -o $(TESTDIR)/test-cosmo $(TESTDIR)/test-cosmo.cpp -lm
$(TESTDIR)/test-scinv: $(TESTDIR)/test-scinv.o angDist.o sigmaCritInv.o
	- $(CC) -o $(TESTDIR)/test-scinv $(TESTDIR)/test-scinv.cpp angDist.o sigmaCritInv.o -lm


# Implicit rules will use CC above to compile these

# All objects depend on these
${OBJLIST}: lens_constants.h
${OBJLIST}: types.h

# some external dependencies
sigmaCritInv.o LensSource.o: angDist.h
objshear.o objshear_pixel.o: LensSource.h
LensSource.o objshear.o objshear_pixel.o: sigmaCritInv.h

# The obvious ones
objshear.o: objshear.cpp

testscrit.o: testscrit.cpp
testscritmean.o: testscritmean.cpp
testscrit_interp2d.o: testscrit_interp2d.cpp

LensSource.o: LensSource.cpp LensSource.h

sigmaCritInv.o: sigmaCritInv.cpp sigmaCritInv.h
angDist.o: angDist.cpp angDist.h

