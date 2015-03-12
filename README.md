# basic_stamp_linux64
Code for the Parallax Basic Stamp, compilable on 64 bit linux
===

### Background

I recently got a Parallax Inc Basic Stamp 2 Experiment Board (AKA the HomeWork Board)
from Radio Shack, on sale as the venerable electronics chain goes into Chapter 11
bankrupcy.  I was disappointed to learn that there was nothing on the company's website
that allowed me to run the board with my Linux machine. 
(https://www.parallax.com/product/555-28188) (Nor was I able to get the Mac version to
run, but maybe I didn't try hard enough.)

### State of affairs

The parallax.com site did link to some assets for the Basic Stamp on Linux:

* https://www.parallax.com/downloads/pbasic-tokenizer (a zip file containing a sample program)

* https://www.parallax.com/downloads/pbasic-editor-linux-part-1 (PBasic compiler part 1)

* https://www.parallax.com/downloads/pbasic-editor-linux-part-2 (library files)

* https://www.parallax.com/downloads/basic-stamp-linux-solution (which gets stampbc_124, v 1.2.4)

Further searches revealed a few other resources:

* http://bstamp.sourceforge.net/ (Basic Stamp tools for Linux, v 2006_05_31)

* http://stampbc.sourceforge.net/ (Stamp Basic Compiler for Linux -
same as what's on the Parallax site, but v 2.1)

* http://ptoke.sourceforge.net/ (Ptoke, another compiler + interface)

* http://parallaxutiliti.sourceforge.net/ (Parallax Utilities, in Java,
seemingly abandoned in the planning stage)

It is readily apparent that no one has been doing much on these for several years at least.
None can be used on a modern Linux machine as distributed.  There are several problems

* The PBasic Compiler solution cannot be built.  It requires the Kylix 3 compiler, a
long-since-abandoned Borland product.  (https://en.wikipedia.org/wiki/Kylix_%28software%29)
Perhaps someone could port this to gcc or some other modern compiler, but I did not attempt
to undertake this.  (I likewise did not look at Parallax Utilities, since it wasn't finished.)

* The tokenizer.so library, on which all of these are based, is a 32 bit library.  This requires
that the programs be built as 32-bit executables, something that is not automatic on normal,
modern Linux distributions.

* The programs all assume that the device will be hooked up to a serial port.  In fact, the board
I purchased comes with a USB connector.  Modern machines generally do not have RS 232 serial ports
on them.

### Goal of the project

My goal here was simply to allow these programs to compile, without warnings, and run.  Aside from
removing obvious problems (generally those that the compiler called to my attention) I did not
attempt to improve anything.  Along the way I made the following changes:

* I removed any defaults to /dev/ttyS0, since users are unlikely to be on that device.  On
Ubuntu, plugging the board in causes the OS to add /dev/ttyUSB0, so this is the new default device.

* These programs use two different versions of the tokenizer.so library: the older v1.16 and
the "current" v1.23.  To make it clearer which one is which, I have renamed them tokenizer_1.16.so
and tokenizer_1.23.so.

* Since the tokenizer.so library is a 32-bit, it seems inappropriate to place it in one of
the system library directories, as these program recommend.  Because it is so old, there is little
likelihood that this "shared object" will in fact be shared with anything.  Instead I wrote an opener
function that looks in /usr/local/basic-stamp/lib followed by . followed by the system directories.

* The showtty command of the two versions of stampbc runs by default in an xterm.  Since xterm is
not generally part of any modern Linux install (though it is easy to add) and because in general
the user would probably prefer to run it (if at all) in a separate tab of the desktop's native
terminal application (e.g. Konsole on KDE), I have made the default be the noterm option.

A note I would add here is that I am including both the stampbc v2.1 and 1.4.2.  This is largely
because I had already done everything on 1.4.2 before I discovered the existence of 2.1 (why is
Parallax distributing the older version?), but also in case there is someone who for some reason
actually wants the older version.

### Prerequisites

My machine is running Kubuntu 14.10, and I have installed gcc.  Since these programs have to be
built as 32 bit executables, some additional libraries have to be installed.  This page
http://www.cyberciti.biz/tips/compile-32bit-application-using-gcc-64-bit-linux.html pointed me
in the right direction.  One has to add the -m32 flag to all compile/link steps, and this
required me to modify the Makefiles.  Plus, 32-bit glibc is needed.  On Debian/Ubuntu, one does
```
sudo apt-get install g++-multilib libc6-dev-i386
```
which will install more than 20 other libraries.  See the article for the relevant installs on
other Linux distributions.

The README in stampbc_124 discusses the possibility of running the system as an IDE under the
editor nedit.  This is a motif-based plain text editor, and a PBASIC syntax highlighter for it
is supplied. Running it reminded me of my work programming in the mid-1990s... Presumably
because nedit is indeed quite outdated, no reference to it is made in version 2.1.  However,
it is, remarkably, available in the Ubuntu repos, so it can be installed with
```
sudo apt-get install nedit
```

When the board is plugged in to a USB port on my machine, it shows up as /dev/ttyUSB0.  The
mode for this file is 660, and the group is "dialout".  To usefully run any of these programs,
therefore, the user should be in this group:
```
sudo usermod -a -G dialout
```

### Future

Having done all of this, I'll probably leave it alone.  Where I have it now is sufficient for
my purposes, which is just to use the board.  I would be happiest if the maintainers
of these projects took this code off my hands, and incorporated it into what they are
distributing.  I see that ptoke is still using the 1.16 version of the .so - perhaps someone
would update it to use 1.23.  I don't have time to improve these programs, but perhaps
someone else will.  Actually, it probably wouldn't be all that hard to create a full-on IDE.

As for Parallax, my hopes are:

* That they release a 64 bit version of their .so so none of this is necessary.

* Failing that, that they copy this stuff onto their web site so users can get it from them.

* That my doing this prompts someone at Parallax spend just a little bit of time on this
stuff.  It did not take me very long to bring this software up to date.  Unlike many other
spheres, the hacker community (the sorts of people who will buy boards like these) are not
infrequently Linux users.

### License

I, the author, hereby release this document, and all of my work on the associated software
into the public domain.
