/* Try to open tokenizer so file */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dlfcn.h>
#ifdef __cplusplus
#include <iostream>
#endif

#include "open_tokenizer_so.h"

#define N_PATHS 3
#define BUFFERSIZE 100

void * open_tokenizer_so(const char * tokenizer_so) {
  static const char * paths[N_PATHS] = {"/usr/local/basic_stamp/lib/", "./", ""};

  void * hso;

  int i;
  for(i = 0; i < N_PATHS; i++) {
    char fileName[BUFFERSIZE+1];
    fileName[BUFFERSIZE] = '\0';
    strncpy(fileName, paths[i], BUFFERSIZE);
    strncat(fileName, tokenizer_so, BUFFERSIZE);
    hso = dlopen(fileName, RTLD_LAZY);
    if(hso) {break;}
  }
  if(!hso) {
    fprintf(stderr, "Can't open %s\n", tokenizer_so);
    perror(dlerror());
    exit(EXIT_FAILURE);
  }

  return hso;
}
