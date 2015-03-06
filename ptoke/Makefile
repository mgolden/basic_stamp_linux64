# Makefile for ptoke
VERSION=0.35a
CC=gcc
LD=gcc
RM=rm
INSTALL=install

INCLUDES=
LIB_DIRS=
LIBS=
LDFLAGS=-L/usr/local/lib -ltoken
CFLAGS=
BIN_DIR=/usr/local/bin
MAN_DIR=/usr/local/man

COMPILE=$(CC) $(INCLUDES) $(CFLAGS) -c $<
LINK=$(LD) $(LDFLAGS) $(LIB_DIRS) $(LIBS)

MTARGET=ptoke
OBJECTS=token.o port.o blast.o

all: ptoke pblast

ptoke: token.o
	$(LINK) -o ptoke token.o

pblast: blast.o port.o
	$(LINK)	-o pblast blast.o port.o

token.o: token.c token.h tokenizer.h
	$(COMPILE)

port.o: port.c port.h
	$(COMPILE)

blast.o: blast.c port.h token.h
	$(COMPILE)

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
