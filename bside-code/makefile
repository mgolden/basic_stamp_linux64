# Makefile for BSide
#
# June 15, 2006

CFLAGS=-Wall -m32

BIN_PREFIX=/usr/local/basic_stamp/bin
LIB_PREFIX=/usr/local/basic_stamp/lib
MAN_DIR = /usr/share/man/man1

all:	bside_tokenizer bside

install:
	cp bside_tokenizer $(BIN_PREFIX)/
	chmod a+rx,g-w,o-w $(BIN_PREFIX)/bside_tokenizer
	cp bside $(BIN_PREFIX)/
	chmod a+rx,g-w,o-w $(BIN_PREFIX)/bside
	cp tokenizer_1.23.so $(LIB_PREFIX)
	chmod a+rx,g-w,o-w $(LIB_PREFIX)/tokenizer_1.23.so
	# Install the man Page
	gzip bside.1
	cp bside.1.gz $(MAN_DIR)/

uninstall:
	rm $(BIN_PREFIX)/bside_tokenizer
	rm $(BIN_PREFIX)/bside
	rm $(LIB_PREFIX)/tokenizer_1.23.so
	rm $(MAN_DIR)/bside.1.gz
	

clean:	
	-rm bside
	-rm bside_tokenizer
	-rm *.o
	-gzip -d bside.1.gz
	
bside:	bside.o stamp.o Parallax.o Coridium.o
	$(CXX) $(CFLAGS) Parallax.o stamp.o Coridium.o bside.o -o bside

bside.o:	stamp.h bside.cpp
	$(CXX) -c $(CFLAGS) bside.cpp

stamp.o:	Parallax.h Coridium.h stamp.h stamp.cpp 
	$(CXX) -c $(CFLAGS) stamp.cpp

Parallax.o:	Parallax.h Parallax.cpp
	$(CXX) -c $(CFLAGS) Parallax.cpp

Coridium.o:	Coridium.h Coridium.cpp
	$(CXX) -c $(CFLAGS) Coridium.cpp

bside_tokenizer:	bside_tokenizer.o open_tokenizer_so.o
	$(CXX) $(CFLAGS) bside_tokenizer.o open_tokenizer_so.o -o bside_tokenizer -ldl

bside_tokenizer.o:	bside_tokenizer.cpp tokenizer.h
	$(CXX) -c $(CFLAGS) bside_tokenizer.cpp

open_tokenizer_so.o: open_tokenizer_so.cpp open_tokenizer_so.h
	$(CXX) -c $(CFLAGS) open_tokenizer_so.cpp
