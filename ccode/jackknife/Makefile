CC = gcc
CFLAGS = -std=c99 -O2 -Wall 
LDFLAGS = -lm

EXEC = jackknife
TABLEFILE = jackknife.table

prefix := /usr/local

default: $(EXEC)

install: $(EXEC)
	mkdir -p $(prefix)/bin
	mkdir -p $(prefix)/ups

	cp $(EXEC) $(prefix)/bin/
	chmod a+x $(prefix)/bin/$(EXEC)

	cp $(TABLEFILE) $(prefix)/ups/

clean: 
	rm -f *.o $(EXEC)


$(EXEC): $(EXEC).c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(EXEC) $(EXEC).c

