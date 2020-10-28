PWD=$(shell pwd)
CC=gcc
CFLAGS=-c -Wall -O -g -I $(DEPS)/libevent/include
OPTIONS=-o 
LIBEVENT=-I $(DEPS)/libevent/include -L $(DEPS)/libevent/lib -levent
DEPS=$(PWD)/deps
ODIR=$(PWD)/obj
SDIR=$(PWD)/src
BDIR=$(PWD)/release
TEST=$(SDIR)/test


all: test_event test_logger0
test_event: $(ODIR)/test_event.o
	$(CC) $(OPTIONS) $(BDIR)/test_event $^ $(LIBEVENT)
test_logger0: $(ODIR)/test_logger0.o $(ODIR)/logger0.o $(ODIR)/util.o
	$(CC) $(OPTIONS) $(BDIR)/test_logger0 $^ 

$(ODIR)/test_event.o: $(TEST)/test_event.c
	$(CC) $(CFLAGS) $< $(OPTIONS) $@ $(LIBEVENT) 
$(ODIR)/test_logger0.o: $(TEST)/test_logger0.c
	$(CC) $(CFLAGS) $< $(OPTIONS) $@
$(ODIR)/logger0.o: $(SDIR)/logger0.c
	$(CC) $(CFLAGS) $< $(OPTIONS) $@
$(ODIR)/util.o: $(SDIR)/util.c
	$(CC) $(CFLAGS) $< $(OPTIONS) $@
.PHONY: clean
clean:
	rm -f $(ODIR)/*.o $(BDIR)/*