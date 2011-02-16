CC	=	g++
#CCFLAGS	=	-O6 -Wall -I../stomp -L../stomp -lstomp
CCFLAGS	=	-O6 -Wall -I../src
LD	=	g++

AR		=	ar
ARFLAGS	=	rv 

OBJS	=	des_stomp.o
LIBRARY	=	libdes_stomp.a

TEST	=	destest
TESTPY	=	destest.py
TESTPRO	=	destest.pro

default: $(LIBRARY)	
test: $(TEST)
all:	$(LIBRARY) $(TEST)

clean:
	rm -f $(LIBRARY) $(OBJS) $(TEST) test_output/*

# OBJS are built automatically
$(LIBRARY): $(OBJS) 
	$(AR) $(ARFLAGS) $@ $(OBJS)
	
$(TEST): $(LIBRARY) destest.cc $(TESTPY) $(TESTPRO)
	$(CC) destest.cc $(CCFLAGS) -lm -L../src -lstomp -L./ -ldes_stomp -o $@
	cp $(TEST) ../bin
	cp $(TESTPY) ../bin
	cp $(TESTPRO) ../bin


.cc.o:
	@ echo compiling $<
	${CC} -c ${CCFLAGS} $<

