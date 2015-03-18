/*
   bside_tokenize.cpp

   This interfaces with Parallax, Inc.'s PBASIC Tokenizer Shared Library for
   the Linux operating system.

   It reads a PBASIC source file and writes the tokenized results to a file
   which can then be sent to the BASIC Stamp microcontroller using the
   "bstamp_run" program.

   Based on "stampapp.cpp" example from the "PBASIC_Tokenizer.tar.gz" archive,
   (c) 2002 Parallax, Inc.  All rights reserved.
   http://www.parallax.com/

   PBASIC is a trademark and BASIC Stamp is a registered trademark of
   Parallax, Inc.
*/

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "tokenizer.h"

/* Globals: */
TModuleRec *ModuleRec;
char Source[MaxSourceSize];
int fd;
float bside_tokenizer_ver = 0.02;
float PBasic_tokenizer_ver;

/* Prototypes */
int write_file(char *filename);
void handle_args(int argsc, char *arr[]);
void print_usage(char *args[]);

int main(int argc, char *args[])
{
	int i;
	void *hso;
	char *error;
	 
	/* Check for proper arguments: */
	handle_args(argc, args);

	/* (Function prototypes): */
	STDAPI (*GetVersion)(void);	
	STDAPI (*CompileIt)(TModuleRec *, char *Src, bool DirectivesOnly, bool ParseStampDirectives, TSrcTokReference *Ref);
	STDAPI (*doTestRecAlignment)(TModuleRec *);
	  
	/* (Open the .so; TO DO: first try in $LD_LIBRARY_PATH ?) */
	   
	hso = dlopen("/usr/local/lib/libbsidetokenizer.so", RTLD_LAZY);
	 
	if (!hso)
	{
		perror(dlerror());
		printf("\nTrying local directory...");

		hso = dlopen("./tokenizer.so", RTLD_LAZY);
		if (!hso)
		{
			perror(dlerror());
			exit(EXIT_FAILURE);
		}
		printf("FOUND!\n");
	}
	  
	/* (Map functions in tokenizer.so) */

	GetVersion= (STDAPI(*)(void))dlsym(hso,"Version");
	CompileIt= (STDAPI(*)(TModuleRec *, char *Src, bool DirectivesOnly,bool ParseStampDirectives, TSrcTokReference *Ref))dlsym(hso,"Compile");
	doTestRecAlignment= (STDAPI(*)(TModuleRec *))dlsym(hso,"TestRecAlignment");
	 
	  /* (Test for any errors) */ 
	  
	error = dlerror();
	  
	if (error != NULL)
	{
		perror(error);
		dlclose(hso);
		exit(EXIT_FAILURE);
	}
	  
	/* Allocate TModuleRec */
	ModuleRec = (TModuleRec *)malloc(sizeof(TModuleRec));

	/* Display version of tokenizer */
	PBasic_tokenizer_ver = (float)(GetVersion()/100);
	printf("PBASIC Tokenizer Library version %1.2f\n\n", PBasic_tokenizer_ver);


	/* Call TestRecAlignment and display the results of the TModuleRec fields. */
	doTestRecAlignment(ModuleRec);
	 
	/* Load source code from file: */ 
	fd = open(args[1], O_RDONLY);
	if (fd == -1)
	{
		printf("Can't open %s\n", args[1]);
		exit(EXIT_FAILURE);
	}

	ModuleRec->SourceSize = read(fd, Source, sizeof(Source));
	Source[ModuleRec->SourceSize] = '\0';
	  
	close(fd);

	/* Tokenize the code: */
	CompileIt(ModuleRec, Source, false, true,NULL);

	/* Close shared library: */
	dlclose(hso);
	 
	/* Did it succeed? */
	if (!ModuleRec->Succeeded)
	{
		printf("Error: Tokenization of %s failed.\n", args[1]);
		printf("Failed compile: %s\n", ModuleRec->Error);
		printf("Error in :\n\t ");
		
		for (i=0; i < ModuleRec->ErrorLength; i++)
		{
			putchar( (int) *(Source + ModuleRec->ErrorStart + i) );
		}
		
		putchar((int) '\n');
		exit(EXIT_FAILURE);
	}

	printf("Tokenized Succesfully!\n");

	if(write_file(args[2]))
	{
		printf("Error: Can't create %s\n", args[2]);
		exit(EXIT_FAILURE);
	}

	printf("%s created succesfully!\n", args[2]);
	  
	/* All Done */
	return 0;
}

/* Save the tokenized results to a file: */
int write_file(char *filename)
{

	fd = open(filename, O_CREAT | O_TRUNC | O_WRONLY, 0644);

	if (fd == -1)
	{	  
		return 1;
	}

	write(fd, ModuleRec->PacketBuffer, ModuleRec->PacketCount * 18);

	close(fd);

	return 0;
}
/* Processes all the command line arguments */
void handle_args(int argsc, char *args[])
{
	int c;
	
	if(argsc > 2)
	{
		 while ( (c = getopt(argsc, args, "hv")) != -1)
		 {
			 switch(c)
			 {
				case 'h':
				case '?':
						print_usage(args);
						exit(EXIT_SUCCESS);
						break;
				case 'v':
						printf("Bside-tokenizer Version: %.2f\n", bside_tokenizer_ver);
						exit(EXIT_SUCCESS);
						break;
				default:
						print_usage(args);
						exit(EXIT_FAILURE);
						break;
			 }
		 }
	 }
	else
	{
		print_usage(args);
		exit(EXIT_SUCCESS);
	}
}

void print_usage (char *args[]) {
	printf("Usage: %s {[OPTION] | infile.txt outfile.tok}\n\n", args[0]);
	printf("Options\n");
	printf("\t-h\t this screen\n");
	printf("\t-v\t print version information\n");
	return;
}

	
