# Makefile for BSide
#
# June 15, 2006

CFLAGS=-Wall

BIN_PREFIX=/usr/local/bin
LIB_PREFIX=/usr/local/lib
HDR_PREFIX=/usr/local/include
MAN_DIR = /usr/share/man/man1

all:	bside_tokenizer bside

install:
	cp bside_tokenizer $(BIN_PREFIX)/
	chmod a+rx,g-w,o-w $(BIN_PREFIX)/bside_tokenizer
	cp bside $(BIN_PREFIX)/
	chmod a+rx,g-w,o-w $(BIN_PREFIX)/bside
	cp tokenizer.so $(LIB_PREFIX)/libbsidetokenizer.so
	chmod a+r,g-w,o-w $(LIB_PREFIX)/libbsidetokenizer.so
	ldconfig
	cp tokenizer.h $(HDR_PREFIX)/bsidetokenizer.h
	chmod a+r,g-w,o-w $(HDR_PREFIX)/bsidetokenizer.h
	# Install the man Page
	gzip bside.1
	cp bside.1.gz $(MAN_DIR)/

uninstall:
	rm $(BIN_PREFIX)/bside_tokenizer
	rm $(BIN_PREFIX)/bside
	rm $(LIB_PREFIX)/libbsidetokenizer.so
	rm $(HDR_PREFIX)/bsidetokenizer.h
	rm $(MAN_DIR)/bside.1.gz
	

clean:	
	-rm bside
	-rm bside_tokenizer
	-rm *.o
	-gzip -d bside.1.gz
	
bside:	bside.o stamp.o Parallax.o Coridium.o
	$(CXX) $(CFLAGS) Parallax.o stamp.o Coridium.o bside.o -o bside

bside.o:	stamp.h bside.cpp

stamp.o:	Parallax.h Coridium.h stamp.h stamp.cpp 

Parallax.o:	Parallax.h Parallax.cpp

Coridium.o:	Coridium.h Coridium.cpp

bside_tokenizer:	bside_tokenizer.o
	$(CXX) $(CFLAGS) bside_tokenizer.o -o bside_tokenizer -ldl

bside_tokenizer.o:	bside_tokenizer.cpp tokenizer.h

