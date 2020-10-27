CC=gcc
CFLAGS=-c -Wall -O -g
OPTIONS=-o 
LIBEVENT=-I $(DEPS)/libevent/include -L $(DEPS)/libevent/lib -levent
DEPS=./deps
ODIR=./obj
SDIR=./src
BDIR=./release
TEST=$(SDIR)/test

all: test_event 
test_event: $(ODIR)/test_event.o
	$(CC) $(OPTIONS) $(BDIR)/test_event $^ $(LIBEVENT)

$(ODIR)/test_event.o: $(TEST)/test_event.c
	$(CC) $(CFLAGS) $< $(OPTIONS) $@ $(LIBEVENT) 

.PHONY: clean
clean:
	rm -f $(ODIR)/*.o $(BDIR)/*