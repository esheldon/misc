CC = gcc
CFLAGS = -std=c99 -O2 -Wall 
LDFLAGS = -lm

EXEC = jackknife

prefix := /usr/local

default: $(EXEC)

install: $(EXEC)
	mkdir -p $(prefix)/bin
	cp $(EXEC) $(prefix)/bin/
	chmod a+x $(prefix)/bin/$(EXEC)

clean: 
	rm -f *.o $(EXEC)


$(EXEC): $(EXEC).c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(EXEC) $(EXEC).c

