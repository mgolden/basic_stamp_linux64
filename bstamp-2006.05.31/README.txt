bstamp

A collection of command-line tools to work with BASIC Stamp microcontrollers
and PBASIC under Linux.

by Bill Kendrick
bill@newbreedsoftware.com
http://www.newbreedsoftware.com

Brian Lavender
brian@brie.com
http://www.brie.com/brian/

Francis Esmonde-White
francis@esmonde-white.com
http://www.esmonde-white.com

PBASIC tokenizer library by, and some code based on examples from,
Parallax, Inc.  Copyright (c) 2002.  All rights reserved.
http://www.parallax.com/

PBASIC is a trademark and BASIC Stamp is a registered trademark of
Parallax, Inc.


May 13, 2004


About:
------
  This collection of tools lets you take a PBASIC source program,
  convert it into the tokenized binary code used by a BASIC Stamp
  microcontroller, and then send that code to your BASIC Stamp device
  over a serial port.


Compiling:
----------
  You should simply be able to run "make" to compile the tools from source.


Installing:
-----------
  the following commands will allow you to install the bstamp utilities:

  Extract the bstamp program source code to a directory, and move into
  directory.

  > make clean
  > make
  > make install

  if necessary, make the com1 serial port
  > MAKEDEV /dev/ttyS0
  or, make the com2 serial port
  > MAKEDEV /dev/ttyS1

  Then you must make a symbolic link to the proper serial port.
  This can be done with:

  > ln -s /dev/ttyS0 /dev/bstamp

How it Works:
-------------
  First, create a program for your BASIC Stamp using the PBASIC language.
  For example:

    '{$STAMP BS2}
    DEBUG "Hello world", CR
    LOOP:
      HIGH 0
      PAUSE 1000
      LOW 0
      PAUSE 1000
      GOTO LOOP

  (This writes the string "Hello world" out to the serial port, for your
  desktop computer to see, and then proceeds to toggle the P0 pin on the
  board.  This could, for example, cause an LED to flash.)

  Save the PBASIC code as an plain ASCII text file.  (e.g., "hello.bs2")


  Second, run the "bstamp_tokenize" program on the PBASIC code.
  It will create a new file containing the binary 'tokens' that the
  BASIC Stamp CPU itself understands.
  (e.g., "bstamp_tokenize hello.bs2 hello.tok")


  Finally, send the tokenized code to the "bstamp_run" program.
  It will connect to the BASIC Stamp using the serial port, send the
  program over, and then watch the serial port for any feedback.
  (For example, caused by that "DEBUG "Hello world", CR" line.)
  (e.g., "bstamp_run hello.tok")


  This can be simplified into a single step as:
  > cat hello.bs2 | bstamp_tokenizer | bstamp_run

  The 'cat' command sends the basic stamp program to the bstamp_tokenizer,
  and the tokenizer then sends the program on to the bstamp_run program.


About the Tokenizer Library:
----------------------------
  The tokenizer is by Parallax, Inc., and released as a shared object (.so).
  As of this writing, it is only available as a binary (no source code), and
  only for x86-based Linux systems.

  It is available, along with example code (upon which some of the code here
  is based) and documentation, from this page at Parallax's website:
  
    http://www.parallax.com/html_pages/downloads/tokenizer/tokenizer.asp


About the PBASIC Examples
-------------------------
  There's one example program inside the "pbasic_examples/" subdirectory:
  "hello.bs2"

  Type "make hello.tok" to tokenize the PBASIC code into tokens.

  Type "make run" to send the tokenized file to the BASIC Stamp.

  NOTE: As a shortcut, you can just type "make run" and it will tokenize,
  if needed!  This means if you edit the PBASIC source, you can just type
  "make run" and it will tokenize it and run it all in one step!

