/*  ptoke - Linux Basic Stamp Burning Utility,
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
#include <getopt.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <fcntl.h>

#include "port.h"
#include "token.h"

#define PTOKE_DESC "pblast - Linux Basic Stamp burner\n"


struct BsoHeader Header;
struct BsoRecord Record;

int Verbose=0;
enum module_type Module=None, Chip=None;
char *Device=NULL;
int filefd;
int Interactive=0;

void printusage(char out);


int main(int argc, char *argv[]){

  int x=0, y=0;

  parse_command(argc, argv);

  prog_init();

  for( x=optind; x<argc; x++){
    loadheader( argv[x] );
    for( y=0; y<Header.RecordCount; y++){
      loadrecord();
      blastrecord();
    }
  }
  p_reset();
  if( Interactive) debug();
  deinit();
}
  

int parse_command( int argc, char *argv[]){

  char c;
  if( argc==1) printusage('h');
  while( (c=getopt( argc, argv, "vhm:d:li"))!=-1){
    switch(c){
    case 'i':
      Interactive=1;
      break;
    case 'l':
      printusage('v');
      break;
    case '?':
      printusage('h');
      break;
    case ':':
      printusage('h');
      break;
    case 'v':
      Verbose=1;
      break;
    case 'm':
      if ( strcmp( optarg, "2" )==0){ Module=bs2; break;}
      if ( strcmp( optarg, "2e" )==0){ Module=bs2e; break;}
      if ( strcmp( optarg, "2sx" )==0){ Module=bs2sx; break;}
      if ( strcmp( optarg, "2p" )==0){ Module=bs2p; break;}
      if ( strcmp( optarg, "2pe" )==0){ Module=bs2pe; break;} 
      error_handle( "Unknown module type in command line", 1, 1);
      break;
    case 'd':
      Device=calloc( strlen(optarg)+1, 1);
      if ( Device==NULL ) error_handle( "Unable to allocate memory", 1, 2);
      strcpy( Device, optarg);
      break;
    case 'h':
      printusage('h');
      break;
    default:
      printusage('h');
      break;
    }
  }
}


int prog_init(){

  char type[10], version[10];

  setvbuf( stdout, NULL, _IONBF, BUFSIZ);

  p_init(Device);

  p_reset();

  Chip=p_ident( type, version);
  if ( Chip==-1) error_handle( "Couldn't ID Chip.", 1, 4);

  if( Verbose==1) printf( "Probed Chip Type: Basic Stamp %s, ver %s\n", type, version);

  if( Module!=None && Module!=Chip){
    error_handle( "Command Line Module type differs from hardware value", 2, 3);
  }

}
  


int loadheader(char *filename){

  int x=0;

  close( filefd);
  filefd=open( filename, O_RDONLY);
  if( filefd==-1) {
    perror("LoadHeader");
    error_handle( "Cannot load object file.", 1, 5);
  }

  x=read( filefd, &Header, sizeof(struct BsoHeader));
  if( x!=sizeof(struct BsoHeader) ){
    perror("Load Header");
    if( x==-1) error_handle( "Cannot load object header.", 1, 6);
    else error_handle( "Object file too short.", 1, 10);
  }
  return(0);
}


int loadrecord(){

  int x=0;

  x=read( filefd, &Record, sizeof( struct BsoRecord));
  if ( x!=sizeof(struct BsoRecord) ){
    perror("Loadrecord");
    if( x==-1) error_handle( "Cannot load object's record.", 1, 7);
    else error_handle( "Object file too short.", 1, 11);
  }

}

int blastrecord(){

  int x=0, y=0, z=0;

  if( Verbose==1)
    fprintf(stderr, "Burning %d packets to slot No. %d\n", Record.PacketCount, Record.SlotNumber);

  p_proginit( Record.SlotNumber);

  for( x=0; x<Record.PacketCount; x++, y+=18){
    z=p_progblast( Record.PacketBuffer+y);
    if( z==1) error_handle("Checksum error during burning packet", 1, 8);
    if( z==2) error_handle("EEPROM error on chip", 1, 9);
    if( x%26==0 && Verbose==1) fprintf(stderr, ".");
  }
  if( Verbose==1) fprintf(stderr,"\n");
  p_halt();
}


int deinit(){

  close(filefd);
  p_deinit();
  if (Verbose==1) fprintf(stderr, "Done.\n");
}



void printusage(char func){
  
  FILE *out;

  if( func=='h') out=stderr;
  if( func=='v') out=stdout;
  fprintf( out, "ptoke version %s\n", PTOKE_VERSION);
  if( func=='v'){
    fprintf(out, GPL);
    exit(0);
  }
  if( func=='h'){
    fprintf( out, "%s%s", PTOKE_DESC, "Usage: pblast [options] file...
      Options:							
      -d <device>      	Use serial device file <device>		
      -h		Print usage
      -i                Enter debug mode after uploading batch
      -l		Print version and copyright info		
      -m <type>	        Specify module type to expect				
      -v		Print verbose output							
		  
For more info see documentation.				
Report bugs to: beretta42@eohio.net				
or see: https://sourceforge.net/projects/ptoke/			
");
    exit(1);
  }
}


int error_handle( char *string, int type, int no){

  if ( type==1) {
    fprintf(stderr, "Fatal Error: %s, No. %d\n", string, no);
    exit(no);
  }

  if( type==2) {
    fprintf(stderr, "Warning: %s, No. %d\n", string, no);
    return(0);
  }
}


int debug(){
  
  fd_set input;
  int x;
  char buf[255];
  int fd;
  
  fd=returnfd();
  
  do{
    FD_ZERO(&input);
    FD_SET(fd, &input);
    x = select(fd+1, &input, NULL, NULL, NULL);      
    if ( x<1) error_handle("Error waiting for debug data", 1, 10);
    if( FD_ISSET( fd, &input) ) {
      x=read(fd, buf, 255);
      buf[x]=0;
      fprintf( stderr, "%s", buf);
    }
  }while(1);
  return(0);
}

