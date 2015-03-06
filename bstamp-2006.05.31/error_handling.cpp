/* Tokenize the code: */
  
  if (CompileIt(ModuleRec, Source, false, true) == false )
  {
	  fprintf(stderr, "Failed compile: %s", ModuleRec->Error);
	  exit(-1);
  }


  /* Close shared library: */
  
  dlclose(hso);
  
  
  // We can remove the following.
  /* Did it succeed? */

  if (!ModuleRec->Succeeded)
  {
    fprintf(stderr, "Error: Tokenization of %s failed.\n", argv[1]);
    exit(1);
  }
  
  
