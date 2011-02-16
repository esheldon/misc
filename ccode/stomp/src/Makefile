CC	=	g++
CCFLAGS	=	-O6 -Wall
LD	=	g++

AR		=	ar
ARFLAGS	=	rv

OBJS	=	stomp_util.o
LIBRARY	=	libstomp.a

TEST	=	stomp_unit_test

TESTDATA	=	RandomPoints.dat StompDensityMap.pix StompMap.pix pixel_circle.map

default: $(LIBRARY)	
test: $(TEST)
all:	$(LIBRARY) $(TEST)

clean:
	rm -f $(TESTDATA) $(LIBRARY) $(OBJS) $(TEST)


# OBJS are built automatically
$(LIBRARY): $(OBJS)
	$(AR) $(ARFLAGS) $@ $(OBJS)
	
$(TEST): $(LIBRARY) stomp_unit_test.cc
	$(CC) stomp_unit_test.cc -lm -O6 -L./ -lstomp -o $@
	cp -f $(TEST) ../bin

stomp_util.o: stomp_util.cc stomp_util.h
	@ echo compiling $<
	${CC} -c ${CCFLAGS} $<
	
