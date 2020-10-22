CC=gcc
CFLAGS=-Wall -O -g
RELEASE=./release
DEPS=./deps
OBJ=./obj
SRC=./src
TEST=$(SRC)/test
$(RELEASE)/testevent: $(OBJ)/testevent.o
	gcc -o $(RELEASE)/testevent $(OBJ)/testevent.o -I $(DEPS)/libevent/include -L $(DEPS)/libevent/lib -levent
$(OBJ)/testevent.o: $(TEST)/testevent.c
	gcc -c $(TEST)/testevent.c -o $(OBJ)/testevent.o -I $(DEPS)/libevent/include -L $(DEPS)/libevent/lib -levent
.PHONY: clean
clean:
	rm $(OBJ)/*.o $(RELEASE)/testevent