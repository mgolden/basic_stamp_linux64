/*
   bstamp_tokenize.cpp

   This interfaces with Parallax, Inc.'s PBASIC Tokenizer Shared Library for
   the Linux operating system.

   It reads a PBASIC source file and writes the tokenized results to a file
   which can then be sent to the BASIC Stamp microcontroller using the
   "bstamp_run" program.

   Based on "stampapp.cpp" example from the "PBASIC_Tokenizer.tar.gz" archive,
   (c) 2002 Parallax, Inc.  All rights reserved.
   http://www.parallax.com/

   Added more UI and stdin/stdout support so that it can be used in a pipe
   Francis Esmonde-White
   francis@esmonde-white.com
   http://www.esmonde-white.com
   May 12, 2004

   Cleaned up and UI added by Bill Kendrick.
   bill@newbreedsoftware.com
   http://www.newbreedsoftware.com/

   April 7, 2003 - April 11, 2003

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

/* --- MAIN --- */

int main(int argc, char * argv[])
{
  int fd,i;
  int io_choice=-1;
  void *hso;
  char *error;


  /* Check for proper arguments: */
  if (argc == 2)
  {
  	fprintf(stdout, "2 arguments!\n");
  	if (argv[1][0]=='-')
	{
		if (argv[1][1]=='h'|argv[1][1]=='?')
		{
			fprintf(stderr, "\nBasic Stamp Linux Tokenizer\n\n");
			fprintf(stderr, "This program is used to tokenize a basic stamp 2 source file.\n");
			fprintf(stderr, "For more information about the Basic Stamp, see http://www.parallax.com\n\n");
			fprintf(stderr, "This is a modified version of bstamp available at http://bstamp.sourceforge.net\n");
			fprintf(stderr, "Options:\n");
			fprintf(stderr, "\t-h or -? : this help listing\n");
			fprintf(stderr, "\t-v : the program version\n");
			fprintf(stderr, "\t-a : about the program (brief history)\n");
			fprintf(stderr, "\tnote: standard UNIX pipes are now implemented!\n");
			fprintf(stderr, "\t  to use stdin do not include any other arguments.\n\n");
			fprintf(stderr, "Standard Usage: bstamp_tokenize INFILE.BS2 OUTFILE.TOK\n");
			fprintf(stderr, " Or:\n cat INFILE.BS2 | bstamp_tokenize | OUTFILE.TOK \n Or:\n cat INFILE.BS2 | bstamp_tokenizer | bstamp_run\n");
			exit(0);
		}
		else if (argv[1][1]=='v')
		{
			fprintf(stderr, "bstamp_tokenizer version 2004-05-12\n current filename: ");
			fprintf(stderr, argv[0]);
			fprintf(stderr, "\n");
			exit(0);
		}
		else if (argv[1][1]=='a')
		{
			fprintf(stderr, "Modified by Francis Esmonde-White [francis@esmonde-white.com] 2004-05-12\n");
			fprintf(stderr, "\tAdded POSIX pipes functionality\n");
			fprintf(stderr, "\tnow using STDIN as the .BS2 file location if no arguments are given\n");
			fprintf(stderr, "\tnow using STDOUT as the .TOK file location if no arguments are given\n");
			exit(0);
		}
		else
		{
			fprintf(stderr, "%s: invalid option -- %s\n", argv[0], argv[1]);
			fprintf(stderr, "Try '%s -h' for more information.\n", argv[0]);
			exit(EXIT_FAILURE);
		}
	}
  }
  else if (argc == 1) // use stdin and stdout
  {
  	io_choice=1;
  }
  else if (argc == 3) // use given input and output files
  {
  	io_choice=2;
  }
  else
  {
	fprintf(stderr, "%s: invalid option -- %s\n", argv[0], argv[1]);
	fprintf(stderr, "%Try '%s -h' for more information.\n", argv[0]);
	exit(EXIT_FAILURE);
  }


  /* Open shared library: */

  /* (Function prototypes): */

  STDAPI (*GetVersion)(void);
  STDAPI (*CompileIt)(TModuleRec *, char *Src, bool DirectivesOnly,
		      bool ParseStampDirectives, TSrcTokReference *Ref);
  STDAPI (*doTestRecAlignment)(TModuleRec *);


  /* (Open the .so; first try in $LD_LIBRARY_PATH) */

  hso = dlopen("libbstamptokenizer.so", RTLD_LAZY);
  if (!hso)
  {
    perror(dlerror());

    hso = dlopen("./tokenizer.so", RTLD_LAZY);
    if (!hso)
    {
      perror(dlerror());
      exit(EXIT_FAILURE);
    }
  }


  /* (Map functions in tokenizer.so) */

  GetVersion= (STDAPI(*)(void))dlsym(hso,"Version");
  CompileIt= (STDAPI(*)(TModuleRec *, char *Src, bool DirectivesOnly,
			bool ParseStampDirectives, TSrcTokReference *Ref))dlsym(hso,"Compile");
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
  fprintf(stderr, "PBASIC Tokenizer Library version %1.2f\n\n",
	  (float)(GetVersion())/100);

  /* Call TestRecAlignment and display the results of the TModuleRec fields. */
  doTestRecAlignment(ModuleRec);


if  (io_choice==1) // IO from stdin and stdout
{
  ModuleRec->SourceSize = fread(Source, 1, sizeof(Source), stdin);
  // ModuleRec->SourceSize = read(fd, Source, sizeof(Source));
  Source[ModuleRec->SourceSize] = '\0';

  /* Tokenize the code: */

  CompileIt(ModuleRec, Source, false, true, NULL);

  /* Close shared library: */

  dlclose(hso);

  /* Did it succeed? */

  if (!ModuleRec->Succeeded)
  {
    	fprintf(stderr, "Error: Tokenization of %s failed.\n", argv[1]);
	fprintf(stderr, "Failed compile: %s\n", ModuleRec->Error);
	fprintf(stderr, "The following text gave the error: ");
	for (i=0; i < ModuleRec->ErrorLength; i++)
		putchar( (int) *(Source + ModuleRec->ErrorStart + i) );
	putchar((int) '\n');
    	exit(-1);
  }

  /* Save the tokenized results to a file: */

  fwrite(ModuleRec->PacketBuffer, 1, ModuleRec->PacketCount * 18, stdout);
  // write(fd, ModuleRec->PacketBuffer, ModuleRec->PacketCount * 18);

}
else // IO from files
{
  /* Load source code from file: */

  fd = open(argv[1], O_RDONLY);
  if (fd == -1)
  {
    printf("Can't open %s\n", argv[1]);
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
    	fprintf(stderr, "Error: Tokenization of %s failed.\n", argv[1]);
	fprintf(stderr, "Failed compile: %s\n", ModuleRec->Error);
	fprintf(stderr, "The following text gave the error: ");
	for (i=0; i < ModuleRec->ErrorLength; i++)
		putchar( (int) *(Source + ModuleRec->ErrorStart + i) );
	putchar((int) '\n');
    	exit(-1);
  }


  /* Save the tokenized results to a file: */

  fd = open(argv[2], O_CREAT | O_TRUNC | O_WRONLY, 0644);

  if (fd == -1)
  {
    fprintf(stderr, "Error: Can't create %s\n", argv[2]);
    exit(EXIT_FAILURE);
  }

  write(fd, ModuleRec->PacketBuffer, ModuleRec->PacketCount * 18);

  close(fd);
} // End of file IO


  /* All done! */

  return 0;
}
