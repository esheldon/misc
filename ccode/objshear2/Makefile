CC = g++
LD = g++

TESTDIR		=	./test

HTMDIR		=	./htm

RM			=	rm -f

TESTEXECTMP	= 	$(TESTDIR)/test-lcat    \
				$(TESTDIR)/test-scat    \
				$(TESTDIR)/test-rev     \
				$(TESTDIR)/test-gcirc   \
				$(TESTDIR)/test-cosmo   \
				$(TESTDIR)/test-sigmacrit

TESTEXEC = $(TESTDIR)/test-cosmo   \
		   $(TESTDIR)/test-lcat    \
		   

test: $(TESTEXEC)

clean:
	- ${RM} *.o test/*.o ${EXECUTABLES} $(TESTEXEC)

$(TESTDIR)/test-cosmo: $(TESTDIR)/test-cosmo.o Cosmology.h
	- $(CC) -o $(TESTDIR)/test-cosmo $(TESTDIR)/test-cosmo.cpp -lm

$(TESTDIR)/test-lcat: $(TESTDIR)/test-lcat.o lcat.o .o lcat.h Cosmology.h
	- $(CC) -o $(TESTDIR)/test-lcat $(TESTDIR)/test-lcat.cpp lcat.o -lm
