PWD=$(shell pwd)
CC=gcc
CFLAGS=-c -Wall -O -g -I $(DEPS)/libevent/include
OPTIONS=-o 
LIBEVENT=-I $(DEPS)/libevent/include -L $(DEPS)/libevent/lib -levent
PTHREAD = -lpthread
DEPS=$(PWD)/deps
ODIR=$(PWD)/obj
SDIR=$(PWD)/src
BDIR=$(PWD)/release
TEST=$(SDIR)/test
PARSE = $(SDIR)/parse


all: test_event test_logger0 test_server main
test_event: $(ODIR)/test_event.o
	$(CC) $(OPTIONS) $(BDIR)/test_event $^ $(LIBEVENT)
test_logger0: $(ODIR)/test_logger0.o $(ODIR)/logger0.o $(ODIR)/util.o 
	$(CC) $(OPTIONS) $(BDIR)/test_logger0 $^ 
#test_mem2: $(ODIR)/test_mem2.o 
#	$(CC) $(OPTIONS) $(BDIR)/test_mem2 $^
test_lex: $(ODIR)/test_lex.o $(ODIR)/lex.yy.o
	$(CC) $(OPTIONS) $(BDIR)/test_lex $^ 
test_server: $(ODIR)/test_server.o 
	$(CC) $(OPTIONS) $(BDIR)/test_server $^
main: $(ODIR)/cache.o $(ODIR)/connection.o $(ODIR)/thread.o $(ODIR)/util.o $(ODIR)/daemon.o
	$(CC) $(OPTIONS) $(BDIR)/main $^ $(LIBEVENT) $(PTHREAD)


$(ODIR)/test_event.o: $(TEST)/test_event.c
	$(CC) $(CFLAGS) $< $(OPTIONS) $@ $(LIBEVENT) 
$(ODIR)/test_logger0.o: $(TEST)/test_logger0.c
	$(CC) $(CFLAGS) $< $(OPTIONS) $@
$(ODIR)/logger0.o: $(SDIR)/logger0.c
	$(CC) $(CFLAGS) $< $(OPTIONS) $@
$(ODIR)/util.o: $(SDIR)/util.c
	$(CC) $(CFLAGS) $< $(OPTIONS) $@
#$(ODIR)/mem0.o: $(SDIR)/mem0.c
#	$(CC) $(CFLAGS) $< $(OPTIONS) $@
#$(ODIR)/mem1.o: $(SDIR)/mem1.c
#	$(CC) $(CFLAGS) $< $(OPTIONS) $@
#$(ODIR)/mem2.o: $(SDIR)/mem2.c 
#	$(CC) $(CFLAGS) $< $(OPTIONS) $@
$(ODIR)/cache.o: $(SDIR)/cache.c
	$(CC) $(CFLAGS) $< $(OPTIONS) $@
$(ODIR)/connection.o: $(SDIR)/connection.c
	$(CC) $(CFLAGS) $< $(OPTIONS) $@ $(LIBEVENT)
$(ODIR)/thread.o: $(SDIR)/thread.c
	$(CC) $(CFLAGS) $< $(OPTIONS) $@ $(LIBEVENT)
$(ODIR)/test_lex.o: $(TEST)/test_lex.c
	$(CC) $(CFLAGS) $< $(OPTIONS) $@
$(ODIR)/daemon.o: $(SDIR)/daemon.c
	$(CC) $(CFLAGS) $< $(OPTIONS) $@
$(ODIR)/test_server.o: $(TEST)/test_server.c
	$(CC) $(CFLAGS) $< $(OPTIONS) $@

.PHONY: clean
clean:
	rm -f $(ODIR)/*.o $(BDIR)/* 