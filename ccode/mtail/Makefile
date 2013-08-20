CC = g++
CFLAGS = -O2 -Wall 
LDFLAGS = ${CFLAGS} -lcurses

EXEC = mtail

PREFIX := /usr/local

default: $(EXEC)

install: $(EXEC)
	mkdir -p $(PREFIX)/bin
	cp $(EXEC) $(PREFIX)/bin/
	chmod a+x $(PREFIX)/bin/$(EXEC)

clean: 
	rm -f *.o $(EXEC)


$(EXEC): $(EXEC).c
	#$(CC) $(CFLAGS) $(LDFLAGS) -o $(EXEC) $(EXEC).c
	$(CC) $(CFLAGS) -o $(EXEC) $(EXEC).c $(LDFLAGS) 
