CC = g++
CFLAGS = -O2 -Wall 
LDFLAGS = ${CFLAGS} -lcurses

EXEC = mtail
EXEC_V1 = mtail-v1.0

PREFIX := /usr/local

default: $(EXEC)
v1: $(EXEC_V1)

install: $(EXEC)
	mkdir -p $(PREFIX)/bin
	cp $(EXEC) $(PREFIX)/bin/
	chmod a+x $(PREFIX)/bin/$(EXEC)

clean: 
	rm -f *.o $(EXEC) $(EXEC_V1)


$(EXEC): $(EXEC).c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(EXEC) $(EXEC).c

$(EXEC_V1): $(EXEC_V1).c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(EXEC_V1) $(EXEC_V1).c
