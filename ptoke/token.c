/*  ptoke - Linux pbasic tokenizer
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
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#include <dlfcn.h>

#include "token.h"

#include "open_tokenizer_so.h"

#define PTOKE_DESC "ptoke - Linux pbasic tokenizer\n"

struct TModuleRec Record; /* a place to store an bso record entry */
struct ProgramRecord *ProgramHead=NULL;  /* ptr to head node */ 

char *OutputFileName=NULL;  /* the target's filename */
int Verbose=0; /* a switch for verbosity */
int VeryVerbose=0;
int Module=0; /* which ver stamp to tokenize for */
int TempSlot=0; /* Tmp holder for a stamp slot number */
int UsedSlot=1; /* assists in logic for commandline parsing */
int BigBso=0; /* weather or not to enable > 8 slot bso target files */
int DisableM4=0; /* global val for m4 preprocessing */
char *M4Includedir="./";   

int addfile( char *filename);
struct ProgramRecord *addrecord(struct ProgramRecord **head);
int slottaken(int slot, struct ProgramRecord *head);
int nextslot(struct ProgramRecord *head);
int tokenize();
void displayerror( struct ProgramRecord *ptr);
int createbso();
int error_handle( char *filename, char *message, int type, int code);
int preproc( FILE *tmp, char *filename, struct fileattrib *attrib);
FILE *m4preproc( char *filename);
void printusage(char function);

int initialize(int argc, char *argv[]);
int bseopen(char *filename);
void pwerror(void);

int main(int argc, char *argv[]){

  initialize(argc, argv);
  tokenize();
  createbso();
  error_handle( "ptoke", "Finished", 4, 0);
  return(0);
}



/* sets up the basics, and parce command line
   also throws addfile() a command line filename
   for adding to the "ProgramRecod" llist */
int initialize(int argc, char *argv[]){

  int c=0;
  
  OutputFileName=malloc( strlen("out.bso")+1); /* sets up the default target filename */
  if(OutputFileName==NULL) error_handle( "ptoke", "Cannot allocate memory", 1, 0);
  sprintf( OutputFileName, "out.bso");
  if( argc ==1) printusage( 'h');
  while( (c=getopt( argc, argv, "-o:s:m:vVhbplI:" )) != -1){  /* getopt command line parcer */
    switch (c){
    case 'I':
      M4Includedir=malloc( strlen( optarg)+1 );
      if( M4Includedir==NULL) error_handle( "ptoke", "Cannot allocate memory", 1, 0);
      sprintf( M4Includedir, "%s", optarg);
      break;
    case 'l':
      printusage( 'v');
      break;
    case 'V':
      VeryVerbose=1;
      break;
    case '?':
      printusage( 'h');
      break;
    case 'h':
      printusage( 'h');
      break;
    case 'p':
      DisableM4=1;
      break;
    case 'o':
      free( OutputFileName); 
      OutputFileName=malloc( strlen(optarg)+1 );
      if (OutputFileName==NULL) error_handle( "ptoke", "Cannot allocate memory", 1, 16);
      sprintf( OutputFileName, "%s", optarg);
      break;
    case 's':
      if( sscanf( optarg, "%d", &TempSlot)!=1 ){
	error_handle( "ptoke", "Invalid slot number in command line", 1, 1);
	break;
      }
      if(0>TempSlot || TempSlot>7) {
	error_handle( "ptoke", "Invalid slot number in command line", 1, 2);
      }
      UsedSlot=0;
      break;
    case 'm':
      if( ! strcmp( "2", optarg) ) {Module=2; break;}
      if( ! strcmp( "2e", optarg) ) {Module=3; break;}
      if( ! strcmp( "2sx", optarg) ) {Module=4; break;}
      if( ! strcmp( "2p", optarg) ) {Module=5; break;}
      if( ! strcmp( "2pe", optarg) ) {Module=6; break;}
      error_handle( "ptoke", "Invalid module type in command line", 1, 3);
      break;
    case 'b':
      BigBso=1;
      break;
    case 1:
      error_handle( optarg, "Opening file", 0, 0);
      addfile( optarg);
      break;
    case 'v':
      Verbose=1;
      break;
    }
  }

  /* Open shared library: */

  void * hso = open_tokenizer_so(TOKENIZER_SO);

  /* (Map functions in tokenizer.so) */

  Compile = (STDAPI(*)(TModuleRec *, char *Src, bool DirectivesOnly,
			bool ParseStampDirectives))dlsym(hso,"Compile");


  /* (Test for any errors) */

  char *error = dlerror();

  if (error != NULL)
  {
    perror(error);
    dlclose(hso);
    exit(EXIT_FAILURE);
  }
  return(0);
}


    
/* This func the does brunt of the work.  It adds a bso file
   to the ProgramRecord linked list, decides other factors
   such as proper slot number assignment, module type-ing, etc.
*/
int addfile( char *optarg){

  int fd;
  int x,y;
  byte slot;
  struct BsoHeader header;
  struct BsoRecord record;
  struct ProgramRecord *ptr;
  char s[255];
 
  if ( (fd=open( optarg, O_RDONLY))==-1 ){
    error_handle( optarg, "Unable to open file, skipping", 2, 4);
    return(-1);
  }

  /* load the file header */
  if ( (x=read( fd, &header, sizeof(struct BsoHeader))) ==-1){ 
    error_handle( optarg, "Unable to read file, skipping", 2, 6);
    return(-1);
  }

  /* send this filename to bseopen if it's not a bso file */
  if ( strcmp( header.Signature, "BMG") ) {
    error_handle( optarg, "Loading as Source", 0, 0);
    close( fd);
    return( bseopen( optarg));
  }
  error_handle( optarg, "Loading as Object", 0, 0);
  if ( x<sizeof(struct BsoHeader)) { 
    error_handle( optarg, "Short BSO header, skipping", 2, 7);
    return(-1);
  }
  
  snprintf( s, 255, "Attempting to add %d record(s) for module type %d", 
	    header.RecordCount, header.Module);
  error_handle( optarg, s, 0, 0);

  /* add new node to ProgramRecord, and fill it with data
     from each record in bso file */
  for( y = 0; y < header.RecordCount; y++){
    ptr=addrecord(&ProgramHead);
    if (ptr==NULL) {
      error_handle( optarg, "Unable to allocate memory", 1, 8);
    }
    
    /* I haven't been able to use the FileName field yet */
    ptr->FileName=malloc( strlen( optarg)+1 );
    if(ptr->FileName==NULL){
      error_handle( optarg, "Unable to allocate memory", 1, 9);
    }
    strcpy( ptr->FileName, optarg);
    ptr->FileType=0; /* indicate this record does not need to by tokenized */
    ptr->SourceBuffer=NULL; /* has no tokenizable source */
    ptr->SourcePermBuffer=NULL; /* has no tokenizable source */
    
    /* decide what module type to assign this record */
    if ( Module!=header.Module  && Module!=0) {
      error_handle( optarg, "File module type differs from command line, overriding", 2, 10);
    }
    if (Module!=0){
      ptr->Record.TargetModule=Module;
    }
    else {
      if ( header.Module!=0) ptr->Record.TargetModule=header.Module;
      else {
	ptr->Record.TargetModule=2;
	error_handle( optarg, "File module type is unused set to 2", 2, 11);
      }
    }

    /* load next record from bso source file */
    if ( (x=read( fd, &record, sizeof(struct BsoRecord)))==-1) {
      error_handle( optarg, "Cannot load BSO file (bad format?)", 2, 12 );
      return(-1);
    }
    if (x<sizeof(struct BsoRecord)){
      error_handle( optarg, "Cannot load BSO file (bad fomat/truncated)", 2, 13);
      return(-1);
    }
    
    snprintf( s, 255, "Object No. %d has %d tokenized packets, for slot No. %d"
	      , y, record.PacketCount, record.SlotNumber);
    error_handle( optarg, s, 0, 0);

    /* copy EEPROM, VarCounts, Packetbuffer and Packetcount from
       bso record to new ProgramRecord llist node */
    for (x=0; x<2048; x++){
      ptr->Record.EEPROM[x]=record.EEPROM[x];
      ptr->Record.EEPROMFlags[x]=record.EEPROMFlags[x];
    }
    for (x=0; x<4; x++){
      ptr->Record.VarCounts[x]=record.VarCounts[x];
    }
    for (x=0; x<2304; x++){
      ptr->Record.PacketBuffer[x]=record.PacketBuffer[x];
    }
    ptr->Record.PacketCount=record.PacketCount;
    
    /* decide how to assign this Record's slot number */
    slot=record.SlotNumber; 
    
    if (UsedSlot==0){
      char c[80];
      sprintf(c, "Slot used from command Line of %d", TempSlot);
      error_handle( optarg, c, 0, 14);
      slot=TempSlot;
      UsedSlot=1;
    }
    ptr->SlotNumber=-1;
    if ( slottaken(slot, ProgramHead) ){
      char c[80];
      slot=nextslot(ProgramHead);
      sprintf( c, "Slot already taken, choosing slot no. %d instead", slot);
      error_handle( optarg, c, 2, 15);
    }
    ptr->SlotNumber=slot;
  }
  snprintf( s, 255, "Actually added %d record(s)", y);
  error_handle( optarg, s, 0, 0);
  close(fd);
  return(0);
}


/* the usual */
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
    fprintf( out, "%s%s", PTOKE_DESC,
	     "Usage: ptoke [options] file...\n"
	     "Options:\n"
	     " -b		Allow slot numbers > 7 in object\n"
	     " -h		Print usage\n"
	     " -l		Print version and copyright info\n"
	     " -m <type>	        Specify Module type to tokenize for\n"
	     " -o <file>	        Place output into <file>\n"
	     " -p		Do not preprocess with m4\n"
	     " -s <slot>	        Use <slot> for next source file slot number\n"
	     " -v		Print verbose output\n"
	     " -I <dir>          use <dir> as secondary m4 include dir\n"
	     " -V		Print hyper-verbose output\n"
	     "\n"
	     "For more info see documentation.\n"
	     "Report bugs to: beretta42@eohio.net\n"
	     "or see: https://sourceforge.net/projects/ptoke/\n"
	   );
    exit(1);
  }
}



/* returns 1 if the given slot # is taken by the linked list
   pointed to by head */
int slottaken(int slot, struct ProgramRecord *head){
  
  struct ProgramRecord *current;
  
  if(head==NULL) return(0);
  current=head;
  do{
    if ( current->SlotNumber==slot ) return(1);
    if ( current->NextRecord==NULL ) return(0);
    current=current->NextRecord;
  }while (current->NextRecord!=NULL);
  return(0);
}



/* returns the next logical unassigned slot number from 
   the linked list pointed to by head*/
int nextslot(struct ProgramRecord *head){
  
  struct ProgramRecord *current;
  int x=0;
  
  if(head==NULL) return(0);
  current=head;
  do{
    if(current->SlotNumber>=x) x=current->SlotNumber+1;
    if(current->NextRecord==NULL) return(x);
    current=current->NextRecord;
  }while (current->NextRecord!=NULL);
  return(x);
}



/* returns a point to a newly initialized node and connects
   former tail node to this new node in linked list */
struct ProgramRecord *addrecord(struct ProgramRecord **head){
  
  struct ProgramRecord *ptr;
  struct ProgramRecord *current;
  
  ptr=malloc( sizeof(struct ProgramRecord) );
  if (ptr==NULL) return(NULL);
  ptr->NextRecord=NULL;
  current=*head;
  if (current==NULL){
    *head=ptr;
    return(ptr);
  }
  for( ; current->NextRecord!=NULL; current=current->NextRecord);
  current->NextRecord=ptr;		      
  return(ptr);
}



/* This program is the non-bso format (pbasic source code)
   counterpart to addfile(); */
int bseopen(char *filename){
  
  FILE *tmp;
  struct fileattrib attrib;
  byte slot=0;
  struct ProgramRecord *ptr;
  int sourcesize=0;
  int x,y;
  char s[255];
  
  /* creat a temp file for include's and source preprocessing */ 
  
  attrib.ModuleType=0;
  attrib.SlotNumber=0;
  
  tmp=tmpfile();
  if (tmp==NULL){
    error_handle( filename, "Cannot generate tempory file", 1, 18);
  }
  
  if ( preproc( tmp, filename, &attrib) )
    error_handle( filename, "Unable to process file", 1, 17);

  snprintf( s, 255, "Adding Source for slot No. %d, for module type %d",
	    attrib.SlotNumber, attrib.ModuleType);
  error_handle( filename, s, 0, 0);
  
  sourcesize=ftell(tmp);
  sprintf( s, "source size: %d bytes", sourcesize);
  error_handle( filename, s, 0, 0);
  fseek(tmp, 0L, SEEK_SET);
  if (sourcesize>65535){
    error_handle( filename, "Resulting source file too long, skipping (try -p)", 2, 19);
    return(-1);
  }
  
  ptr=addrecord(&ProgramHead);

  ptr->FileName=malloc( strlen( filename)+1 );
  if(ptr->FileName==NULL){
    error_handle( filename, "Cannot allocate memory", 1, 20);
  }

  strcpy( ptr->FileName, filename);
  
  ptr->FileType=1;
  
  if ( Module!=attrib.ModuleType && Module!=0) {
    error_handle( filename, "File module type differs from commandline, overriding", 2, 21);
  }
  if (Module!=0){
    ptr->Record.TargetModule=Module;
  }
  else {
    if ( attrib.ModuleType!=0) ptr->Record.TargetModule=attrib.ModuleType;
    else {
      ptr->Record.TargetModule=2;
      error_handle( filename, "File module type is unused, set to 2", 2, 22);
    }
  }

  slot=attrib.SlotNumber;
  if (UsedSlot==0){
    error_handle( filename, "Using slot number taken from command line", 0, 23);
    slot=TempSlot;
    UsedSlot=1;
  }
  ptr->SlotNumber=-1;
  if ( slottaken(slot, ProgramHead) ){
    char c[80];
    slot=nextslot(ProgramHead);
    sprintf( c, "Slot No. already taken, using No. %d instead", slot);
    error_handle( filename, c, 2, 24);
  }
  ptr->SlotNumber=slot;
  
  ptr->Record.SourceSize=sourcesize;
  
  ptr->SourceBuffer=calloc( sourcesize+1, 1);
  if (ptr->SourceBuffer ==NULL ) {
    error_handle( filename, "Cannot allocate memory", 1, 25);
  }

  ptr->SourcePermBuffer=calloc( sourcesize+1, 1);
  if (ptr->SourcePermBuffer ==NULL) {
    error_handle( filename, "Cannot allocate memory", 1, 26);
  }
  
  for (x=0; x<sourcesize; x++){
    y=fgetc(tmp);
    ptr->SourceBuffer[x]=y;
    ptr->SourcePermBuffer[x]=y;
  }
  fclose(tmp);
  return(0);
}


int tokenize(){

  struct ProgramRecord *current=ProgramHead;
  
  if(current==NULL){
    error_handle( "ptoke", "No records to tokenize", 1, 27);
  }
  do{
    if (current->FileType==1){
      error_handle( current->FileName, "Tokenizing", 0, 0);
      Compile(&current->Record, current->SourceBuffer, 0, 0);
      if (current->Record.Succeeded==0){
	error_handle( current->FileName, current->Record.Error, 3, 28);
	displayerror( current);
      }
    }
    current=current->NextRecord;
  }while (current!=NULL);
  return(0);
}
  

void displayerror( struct ProgramRecord *ptr){

    int x,y=0, start, finish;

    if( !DisableM4 ) {
      int y=-1, z, w;
      char filename[255];
      x=ptr->Record.ErrorStart;
      do{
	for( ; (ptr->SourcePermBuffer[x]!=10)&&(x>=0); x--);
	y++;
	w=sscanf( &ptr->SourcePermBuffer[x+1], 
		"'line %d ""%s""", &z, filename);
	x--;
      }while( w!=2 );
      fprintf( stderr, "In line no. %d of %s:\n\n", z+y-1, filename);
    }
    else{
      for( x=ptr->Record.ErrorStart; x>=0; x--){
	if( ptr->SourcePermBuffer[x]==10) y++;
      }
      fprintf( stderr, "In line no. %d of %s:\n\n", y+1, ptr->FileName);
    }

    for( x=ptr->Record.ErrorStart; (ptr->SourcePermBuffer[x]!=10)||(x<0); x--);
    x++;
    start=ptr->Record.ErrorStart-x;
    finish=start+ptr->Record.ErrorLength-1;
    
    while( ptr->SourcePermBuffer[x]!=10){
      fprintf(stderr, "%c", ptr->SourcePermBuffer[x]);
      x++;
    }
    fprintf(stderr, "\n");
    for(x=0; x<=finish; x++){
      if( x>=start) fprintf(stderr,"^");
      else fprintf(stderr," ");
    }
    fprintf(stderr, "\n");
    exit(-1);
}


int createbso(){

  struct ProgramRecord *current=ProgramHead;
  int fd;
  byte RecordCount=0;
  byte Module;
  char s[255];
  int x;

  if( current==NULL){
    error_handle( "ptoke", "No tokenized programs to write", 1, 29);
  }
  Module=current->Record.TargetModule;
  do{
    RecordCount++;
    if( Module!=current->Record.TargetModule){
      error_handle( "ptoke", "Tokenized objects are for different module types", 1, 30);
    }
    current=current->NextRecord;
  }while (current!=NULL);

  if( RecordCount>8 && BigBso==0 ) {
    error_handle( OutputFileName, "Too many records, truncating out to 8 ( try -b?)", 2, 38);
  } 

  fd=open( OutputFileName, O_WRONLY|O_CREAT|O_TRUNC, S_IREAD|
	   S_IWRITE);
  if(fd==-1){
    error_handle( "ptoke", "Cannot create out file", 1, 31);
  }

  snprintf( s, 255, "Creating BSO with %d records, for module type %d",
	    RecordCount, Module);
  error_handle( OutputFileName, s, 4, 0);

  if ( write( fd, "BMG", 4)!=4) pwerror();
  if ( write( fd, &Module, 1)!=1) pwerror();
  if ( write( fd, &RecordCount, 1)!=1) pwerror();
  
  current=ProgramHead;
  do{
    snprintf( s, 255, "Record No. %d for slot No. %d, with %d packet(s) containing"
	      , x, current->SlotNumber, current->Record.PacketCount);
    error_handle( OutputFileName, s, 4, 0);
    snprintf( s, 255, "\t %d words\t%d bytes\t%d nibbles\t%d bits",
	      current->Record.VarCounts[3], current->Record.VarCounts[2],
	      current->Record.VarCounts[1], current->Record.VarCounts[0]);
    error_handle( OutputFileName, s, 4, 0);
    if ( write( fd, &current->Record.PacketCount, 1)!=1) pwerror();
    if ( write( fd, current->Record.EEPROM, 2048)!=2048) pwerror();
    if ( write( fd, current->Record.EEPROMFlags, 2048)!=2048) pwerror();
    if ( write( fd, current->Record.VarCounts, 4)!=4) pwerror();
    if ( write( fd, current->Record.PacketBuffer, 2304)!=2304) pwerror();
    if ( write( fd, &current->SlotNumber, 1)!=1) pwerror();
    current=current->NextRecord;  x++;
    if ( x>8 && BigBso==0) return(0);
  }while (current!=NULL);
  return(0);
}

void pwerror(void){
  error_handle( "ptoke", "Cannot write out file", 1, 32);
}


int error_handle( char *filename, char *message, int type, int code){

  if( type==0 && Verbose==1){
    printf("%s: %s\n", filename, message);
    return(0);
  }
  if( type==4 && VeryVerbose==1){
    printf("%s: %s\n", filename, message);
    return(0);
  }
  if( type==1) {
    fprintf(stderr, "%s: Error: [%d] %s\n", filename, code, message);
    exit(code);
  }
  if( type==2) {
    fprintf(stderr, "%s: Warning: [%d] %s\n", filename, code, message);
    return(0);
  }
  if( type==3) {
    fprintf(stderr, "%s: Tokenizer Error: [%d] %s\n", filename, code, message);
    return(0);
  }
  return(0);
}



/* preproc() m4's filename, filters ptoke directives of, and cats
   source to tmp.*/
int preproc( FILE *tmp, char *filename, struct fileattrib *attrib){

  char line[1000];
  char s[1000];
  FILE *file;
  int x;

  file=m4preproc( filename);
  if( file==NULL) return(-1);

  while( fgets( line, 1000, file) ) {
    if( sscanf(line, "'module %s", s) ) {
      char c[80];
      if ( !strcmp(s, "2" ) ){attrib->ModuleType=2;}
      if ( !strcmp(s, "2e" ) ){attrib->ModuleType=3;}
      if ( !strcmp(s, "2sx" ) ){attrib->ModuleType=4;}
      if ( !strcmp(s, "2p" ) ){attrib->ModuleType=5;}
      if ( !strcmp(s, "2pe" ) ){attrib->ModuleType=6;}
      if ( attrib->ModuleType<2 || attrib->ModuleType>6 ){
	sprintf( c, "Unrecognized Module type: %s, skipping", s);
	error_handle( filename, c, 2, 19);
      }
    }
    if( sscanf(line, "'slot %d", &x) ) attrib->SlotNumber=x;
    if( line[0]=='#' ) line[0]=39; /*performs necessary conversion so hashes
				     won't screw up Compile();*/
    fprintf( tmp, "%s", line);
  }
  if ( DisableM4) fclose( file);
  else pclose( file);
  return( 0);
}



FILE *m4preproc( char *filename){

  FILE *tmp;
  char s[255];

  if( DisableM4) {
    tmp=fopen( filename, "r");
    if ( tmp==NULL) {
      error_handle( filename, "Unable to open file, skipping", 2, 33);
      return( NULL);
    }
    return( tmp);
  }  
  snprintf( s, 255, "m4 -I %s -s %s", M4Includedir, filename);
  tmp=popen( s, "r");
  if( tmp==NULL){
    error_handle( "ptoke" , "Unable to open file (m4 missing? try -p)", 2, 34);
    return(NULL);
  }
  return( tmp);
}

