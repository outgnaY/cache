CC=gcc
CFLAGS=-c -Wall -O -g
OPTIONS=-o 
LIBEVENT=-I $(DEPS)/libevent/include -L $(DEPS)/libevent/lib -levent
DEPS=./deps
ODIR=./obj
SDIR=./src
BDIR=./release
TEST=$(SDIR)/test

all: testevent 
testevent: $(ODIR)/testevent.o
	$(CC) $(OPTIONS) $(BDIR)/testevent $^ $(LIBEVENT)

$(ODIR)/testevent.o: $(TEST)/testevent.c
	$(CC) $(CFLAGS) $< $(OPTIONS) $@ $(LIBEVENT) 

.PHONY: clean
clean:
	rm -f $(ODIR)/*.o $(BDIR)/*