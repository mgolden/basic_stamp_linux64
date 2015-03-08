/* This is an example header file for interfacing with the PBASIC Tokenizer Shared Library.
   on the Linux platform.*/
   
#if defined(__cplusplus)
extern "C"
{
#endif

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <dlfcn.h>
#include <limits.h>

/* Simple Defines */
#define True  			1
#define False 			0
#define MaxSourceSize 	0x10000	
#define EEPROMSize 		2048

/* Ensure types are correct size */
#undef byte                         /*Set 'byte' to 8-bits*/
#if UCHAR_MAX == 0xff		        /*and verify 'char is 8-bits*/
  typedef unsigned char byte;
#else
  #error 'byte', 'char' and 'bool' MUST be 8 bits!
#endif

#undef bool                         /*Set 'bool' to 8-bits*/
#define bool byte		            /*Note: byte size is verified above*/

#undef word                         /*Set 'word' to 16-bits*/
#if USHRT_MAX == 0xffff
  typedef unsigned short int word;
#else
  #error 'word' MUST be 16 bits!
#endif

#if INT_MAX != 0x7fffffff           /*Verify 'int' is 32-bits*/
  #if INT_MAX != 0xffffffff
    #error 'int' MUST be 32 bits!
  #endif
#endif

/* Define STDAPI as type for imported function from shared library */
#define STDAPI    uint8_t         	/* define STDAPI */

/*Define source token cross reference structure*/
    struct TSrcTokReference
	    	 {
/*2 bytes*/	 word	SrcStart;
/*2 bytes*/	 word	TokStart;
		 };
  
/*Define module compile object code type*/
	typedef byte   TPacketType[int(EEPROMSize/16*18)];
    struct TModuleRec
                 {
 /*1 byte*/						  bool         Succeeded;				/*Pass or failed on compile*/
 /*4 bytes*/					  char         *Error;					/*Error message if failed*/
 /*1 byte*/						  bool         DebugFlag;				/*Indicates there's debug data*/

 /*1 byte*/						  byte         TargetModule;			/*BASIC Stamp Module to compile for. 0=None, 1=BS1, 2=BS2, 3=BS2e, 4=BS2sx, 5=BS2p, 6=BS2pe*/
 /*4 bytes*/					  int          TargetStart;				/*Beginning of $STAMP directive target, in source*/
 /*4 bytes*/					  char         *ProjectFiles[7];		/*Paths and names of related project files, if any*/
 /*4 bytes*/					  int          ProjectFilesStart[7];	/*Beginning of project file name in source*/

 /*4 bytes*/					  char         *Port;					/*COM port to download to*/
 /*4 bytes*/					  int          PortStart;				/*Beginning of port name in source*/

 /*4 bytes*/					  int	       LanguageVersion;				/*Version of target PBASIC language.  200 = 2.00, 250 = 2.50, etc */
 /*4 bytes*/					  int	       LanguageStart;				/*Beginning of language version in source*/

 /*4 bytes*/					  int          SourceSize;				/*Enter actual source code length here*/
 /*4 bytes*/					  int          ErrorStart;				/*Beginning of error in source*/
 /*4 bytes*/					  int          ErrorLength;				/*Number of bytes in error selection*/

 /*1*EEPROMSize bytes*/			  byte         EEPROM[EEPROMSize];		/*Tokenized data if compile worked*/
 /*1*EEPROMSize bytes*/			  byte         EEPROMFlags[EEPROMSize];	/*EEPROM flags, bit 7: 0=unused, 1=used*/
																		/*bits 0..6: 0=empty, 1=undef data, 2=def data, 3=program*/

 /*1*4 bytes*/					  byte         VarCounts[4];			/*# of.. [0]=bits, [1]=nibbles, [2]=bytes, [3]=words*/
 /*1 byte*/						  byte         PacketCount;				/*# of packets*/
 /*1*(EEPROMSize/16*18) bytes*/   TPacketType  PacketBuffer;			/*packet data*/
                 };

#if defined(__cplusplus)
}
#endif

