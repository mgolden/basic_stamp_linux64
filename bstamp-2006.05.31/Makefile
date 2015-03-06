# Makefile for BASIC Stamp tokenizer and uploader/runner for Linux
#
# by Bill Kendrick
# bill@newbreedsoftware.com
# http://www.newbreedsoftware.com
#
# and Brian Lavender
# brian@brie.com
# http://www.brie.com/
#
# and Francis Esmonde-White
# francis@esmonde-white.com
# http://www.esmonde-white.com
#
# May 13, 2004


CFLAGS=-Wall

BIN_PREFIX=/usr/local/bin
LIB_PREFIX=/usr/local/lib
HDR_PREFIX=/usr/local/include


all:	bstamp_tokenize bstamp_run

install:
	cp bstamp_tokenize $(BIN_PREFIX)/
	chmod a+rx,g-w,o-w $(BIN_PREFIX)/bstamp_tokenize
	cp bstamp_run $(BIN_PREFIX)/
	chmod a+rx,g-w,o-w $(BIN_PREFIX)/bstamp_run
	cp tokenizer.so $(LIB_PREFIX)/libbstamptokenizer.so
	chmod a+r,g-w,o-w $(LIB_PREFIX)/libbstamptokenizer.so
	ldconfig
	cp tokenizer.h $(HDR_PREFIX)/bstamptokenizer.h
	chmod a+r,g-w,o-w $(HDR_PREFIX)/bstamptokenizer.h

uninstall:
	rm $(BIN_PREFIX)/bstamp_tokenize
	rm $(BIN_PREFIX)/bstamp_run
	rm $(LIB_PREFIX)/libbstamptokenizer.so
	rm $(HDR_PREFIX)/bstamptokenizer.h
	

clean:	
	-rm bstamp_run
	-rm bstamp_tokenize
	-rm *.o

bstamp_run:	bstamp_run.o
	$(CXX) $(CFLAGS) bstamp_run.o -o bstamp_run

bstamp_tokenize:	bstamp_tokenize.o
	$(CXX) $(CFLAGS) bstamp_tokenize.o -o bstamp_tokenize -ldl

bstamp_run.o:	bstamp_run.cpp

bstamp_tokenize.o:	bstamp_tokenize.cpp tokenizer.h

