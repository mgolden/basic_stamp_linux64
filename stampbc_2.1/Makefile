.SUFFIXES: .o .cc

EXEC   = stampbc 
OBJS   = stampbc.o SBTokenizer.o SBCompiler.o SBProject.o SBLoader.o
LIBS   = -ldl
CFLAGS = -Wall -D_COLORFUL
#CFLAGS = -g -Wall -D_COLORFUL
CC     = g++ $(CFLAGS)

INSTALLROOT = /usr/local
INSTBINPATH = $(INSTALLROOT)/bin
INSTLIBPATH = $(INSTALLROOT)/lib
INSTSCRIPT  = $(INSTBINPATH)/$(EXEC)
INSTEXEC    = $(INSTBINPATH)/__$(EXEC)

BACKUPTAR   = stampbc_21.tgz

$(EXEC): $(OBJS)
	$(CC) -o $(EXEC) $(OBJS) $(LIBS)

.cc.o:
	$(CC) -c $<

clean:
	rm -f *.o
	rm -f $(EXEC)

install:
# copy lib and executable
	cp tokenizer.so $(INSTLIBPATH)
	cp $(EXEC) $(INSTEXEC)

# write wrapper script
	echo "#!/bin/bash" > $(INSTSCRIPT)
	echo "LD_LIBRARY_PATH=$(INSTLIBPATH) $(INSTEXEC) \$$*" >> $(INSTSCRIPT)
	chmod a+x $(INSTSCRIPT)

uninstall:
	rm -f $(INSTLIBPATH)/tokenizer.so
	rm -f $(INSTSCRIPT)
	rm -f $(INSTEXEC)

tar:
	tar -zcf $(BACKUPTAR) \
	*.h *.cc \
	tokenizer.so \
	test*.bs? \
	README \
	showtty \
	Makefile

