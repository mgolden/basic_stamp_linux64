/*  ptoke - Linux pbasic tokenizer,
    Copyright (C) 2003, Brett M. Gordon

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include <stdio.h>
#include <termios.h>
#include <termio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <strings.h>

#include "port.h"


static int fd;
struct termios term_set;
struct termios old_term;

void p_fatal_error(int error);
int p_read(char *buf);



void p_init(const char *devname){

  int status;

  fd=open(devname, O_RDWR);
  if (fd == -1) p_fatal_error(1);
  
  if( tcgetattr( fd, &old_term ) ) p_fatal_error(2);

  term_set=old_term;
  
  if( cfsetospeed( &term_set, B9600 ) ) p_fatal_error(3);
  if( cfsetispeed( &term_set, B9600 ) ) p_fatal_error(3);

  cfmakeraw( &term_set );
 
  if( tcsetattr ( fd, TCSAFLUSH, &term_set ) ) p_fatal_error(4);

  if( ioctl(fd, TIOCMGET, &status) ) p_fatal_error(5);
  
  status &= ~TIOCM_DTR;
  
  if( ioctl(fd, TIOCMSET, &status) ) p_fatal_error(6);

  /*  if( fcntl(fd, F_SETFL, FNDELAY) ) p_fatal_error(16); */
}



int p_reset(void){

  int status;
  int x=0, y=0;
  char c;

  if( ioctl( fd, TIOCMGET, &status ) ) p_fatal_error(7);

  if( fork() !=0 ) {
    if( tcsendbreak ( fd, 0 ) ) p_fatal_error(8);
    wait(NULL);
    usleep(22000);

    if( ioctl(fd, FIONREAD, &x) ) p_fatal_error(19);

    for( ;y<x; y++){
      if ( p_read(&c) ) return(-1);
    }
  }
  else {

    status |= TIOCM_DTR;

    if( ioctl(fd, TIOCMSET, &status ) ) p_fatal_error(8);

    usleep(3000);

    status &= ~TIOCM_DTR;

    if( ioctl( fd, TIOCMSET, &status ) ) p_fatal_error(9);
    exit(0);

  }
  return(0);
}



enum module_type p_ident(char *type, char *version){

  int x=0, y=0;
  char c;

  if( write( fd, "e", 1) != 1) p_fatal_error(12);
  if( p_read(&c) ) return(-1);
  if( !p_read(&c) ) {
    if( c=='e') {
      sprintf(type, "2e"); sprintf(version, "1.0");
      return(bs2e);
    }
  }

  p_reset();
  if( write(fd, "B", 1) !=1) p_fatal_error(12);
  if( p_read(&c) ) return(-1);
  if( !p_read(&c) ) {
    if( ~(c-1)==190 ) {
      if(write(fd, "S", 1) !=1) p_fatal_error(12);
      if( p_read(&c) ) return(-1);
      if( !p_read(&c) ) {
	if ( ~(c-1)==173 ) {
	  if(write(fd, "2", 1) !=1) p_fatal_error(12);
	  if( p_read(&c) ) return(-1);
	  if( !p_read(&c) ) {
	    if( ~(c-1)==206) {
	      if(write(fd, "\0", 1) !=1) p_fatal_error(12);
	      if(p_read(&c) ) return(-1);
	      if(!p_read(&c) ) {
		sprintf(type, "2"); 
		y=c&15; x=c>>4;
		sprintf(version, "%d.%d", x, y);
		return(bs2);
	      }
	    }
	  }
	}
      }
    }
  }
  return(-1);
}



void p_fatal_error(int error){

  char string[80];
  sprintf(string, "From Port.c: Err No. %d", error);
  perror(string);
  exit(2);
}



int p_read(char *buf){

  fd_set         input;
  struct timeval timeout;
  int x;

  
  FD_ZERO(&input);
  FD_SET(fd, &input);

  timeout.tv_sec  = 3;
  timeout.tv_usec = 0;

  x = select(fd+1, &input, NULL, NULL, &timeout);
      
  if (x < 0) p_fatal_error(14);
  else if (x == 0) return(-1);
  
  if ( read(fd, buf, 1) ==-1 ) p_fatal_error(15);
  return(0);
}  



int p_complete(void){

  char c=0;
  
  if( write(fd, &c, 1)!=1 ) p_fatal_error(20);
  if( p_read(&c) ) return(-1);
  if( c!='\0' ) return(-2);
  return(0);
}



int p_proginit( int progslot){

  char c=0;
  enum module_type chip;
  char type[10], version[10];

  if ( p_reset() ) p_fatal_error(21);

  chip=p_ident(type, version);
  if ( chip==-1) return (-1);
  
  if ( progslot>0 && chip==bs2) return(-2);
  
  if ( chip!=bs2 ) {
    if ( write( fd, &progslot, 1)!=1 ) p_fatal_error(21);
    if ( p_read(&c) ) p_fatal_error(21);
    if ( c!=progslot) p_fatal_error(21);
  }
  return(0);
}



int p_progblast( void *buff){

  char rbuff[19];

  char c=0;
  int x;

  rbuff[19]=0;
  rbuff[0]=0;
  if( write(fd, buff, 18)!=18 ) p_fatal_error(22);
  for( x=0; x<19; x++){
    if( p_read(&c) ) p_fatal_error(22);
    rbuff[x]=c;
  }
  if ( bcmp( rbuff, buff, 18) ) p_fatal_error(22);
  return( (int)rbuff[18] );
}

int p_halt(void){    /* This Doesn't Work, Anyone care to fix? */

  int status;

  ioctl(fd, TIOCMGET, &status);
  status |= TIOCM_DTR;
  if( ioctl(fd, TIOCMSET, &status ) ) p_fatal_error(18);
  return(0);
}
  
void p_deinit(void){

  tcsetattr( fd, TCSAFLUSH, &old_term);
  close(fd);
}

int returnfd(void){
  return(fd);
}

/*
int main(){

  char type[]="Default";
  char version[]="Default";
  int x;

  printf("Testing Sequence\n");
  p_init("/dev/ttyS0");
  printf("Initialized!\n");
  p_reset();
  if ( p_ident(type, version) ) {
    printf("Read Timeout!\n");
  }
  printf("This is a Basic Stamp %s - Version %s\n", type, version);
  x=p_complete();
  if (x==-1) printf("Complete Timeout\n");
  if (x==-2) printf("Complete Weird Echo\n");
  sleep(10);
  printf("Reset Command\n");
  p_reset();
  exit(0);
}
*/





















