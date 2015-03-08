# Makefile for ptoke
VERSION=0.35a
CC=gcc
LD=gcc
RM=rm
INSTALL=install

INCLUDES=
LIB_DIRS=
LIBS=-ldl
LDFLAGS=-m32
CFLAGS=-Wall -m32
BIN_DIR=/usr/local/basic_stamp/bin
MAN_DIR=/usr/local/basic_stamp/man

COMPILE=$(CC) $(INCLUDES) $(CFLAGS) -c $<
LINK=$(LD) $(LDFLAGS) $(LIB_DIRS)

MTARGET=ptoke
OBJECTS=token.o port.o blast.o

all: ptoke pblast

ptoke: token.o open_tokenizer_so.o
	$(LINK) -o ptoke token.o open_tokenizer_so.o $(LIBS)

pblast: blast.o port.o
	$(LINK)	-o pblast blast.o port.o $(LIBS)

token.o: token.c token.h tokenizer.h
	$(COMPILE)

port.o: port.c port.h
	$(COMPILE)

blast.o: blast.c port.h token.h
	$(COMPILE)

open_tokenizer_so.o: open_tokenizer_so.c open_tokenizer_so.h
	$(COMPILE) $(CFLAGS) -c open_tokenizer_so.c

install: ptoke pblast doc/ptoke.1
	$(INSTALL) -m 755 ptoke pblast $(BIN_DIR)
	$(INSTALL) -m 664 doc/ptoke.1 $(MAN_DIR)/man1
	$(INSTALL) -m 664 doc/pblast.1 $(MAN_DIR)/man1
clean:
	$(RM) -f *~ core *.o ptoke pblast

doc/ptoke.1:

tar:
	$(RM) -f *~ core *.o pblast ptoke
	tar -C ../ -czf ../ptoke-$(VERSION).tar.gz ptoke --exclude *CVS*
