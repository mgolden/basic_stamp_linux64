# Makefile for example PBASIC stuff...
#
# Bill Kendrick
# bill@newbreedsoftware.com
#
# April 7, 2003 - April 16, 2003


all:	hello.tok touch.tok

clean:
	-rm hello.tok
	-rm touch.tok

hello.tok:	hello.bs2
	bstamp_tokenize hello.bs2 hello.tok

touch.tok:	touch.bs2
	bstamp_tokenize touch.bs2 touch.tok
