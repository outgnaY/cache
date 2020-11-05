PWD=$(shell pwd)
CC=gcc
CFLAGS=-c -Wall -O -g 
OPTIONS=-o 
ODIR=$(PWD)/../../obj


$(ODIR)/lex.yy.o: lex.yy.c
	gcc -g -c lex.yy.c
lex.yy.c: scan.l
	flex scan.l
clean:
	rm -f lex.yy.c