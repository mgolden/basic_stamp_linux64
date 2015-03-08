/* This file contains excerts from the orginal tokenizer.h
   as released by Parallax Inc.'s tokenizer lib. */
  
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <dlfcn.h>
#include <limits.h>
  

  /* Ensure types are correct size */
#undef byte                         
#if UCHAR_MAX == 0xff		        
  typedef unsigned char byte;
#else
#error 'byte', 'char' and 'bool' MUST be 8 bits!
#endif
  
#undef bool                         
#define bool byte		            

struct TModuleRec
{
  bool Succeeded;           /*Pass or failed on compile*/
  char *Error;		    /*Error message if failed*/
  bool DebugFlag;	    /*Indicates there's debug data*/
 
  byte TargetModule;			
  int TargetStart;	    /*Beginning of $STAMP directive target, in source*/
  char *ProjectFiles[7];    /*Paths and names of related project files, if any*/
  int ProjectFilesStart[7];/*Beginning of project file name in source*/
  
  char *Port;		    /*COM port to download to*/
  int PortStart;	    /*Beginning of port name in source*/
  
  int SourceSize;	    /*Enter actual source code length here*/
  int ErrorStart;	    /*Beginning of error in source*/
  int ErrorLength;	    /*Number of bytes in error selection*/
  
  byte EEPROM[2048];	    /*Tokenized data if compile worked*/
  byte EEPROMFlags[2048];	
  byte VarCounts[4];	    /*# of.. [0]=bits, [1]=nibbles, [2]=bytes, [3]=words*/
  byte PacketCount;	    /*# of packets*/
  byte PacketBuffer[2304];  /*packet data*/
};
  
typedef struct TModuleRec TModuleRec;

/* Define STDAPI as type for imported function from shared library */
#define STDAPI    uint8_t         	/* define STDAPI */

/* Function prototype for function included from .so */
STDAPI (*Compile)(TModuleRec *, char *Src, bool DirectivesOnly,
		      bool ParseStampDirectives);

#define TOKENIZER_SO "tokenizer_1.16.so"
