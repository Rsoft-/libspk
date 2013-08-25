/*
 * spkx.c - SPK extractor
 * Extracts a single file from an SPK package
 * Copyright (c) 2013 René »rsoft« Ilgner
 * Licensed under GPLv3, see LICENSE for further details
 */ 

#include <stdio.h>
#include <stdlib.h>

#include "libspk.h"

#ifndef EXIT_FAILURE
#define EXIT_FAILURE 1
#endif

#ifndef EXIT_SUCCESS
#define EXIT_SUCCESS 0
#endif

int main(int argc, char** argv) {
  struct spkState state;
  struct spkFileHandle* readFile;
  FILE* writeFile;
  char readBuffer[1024];
  char useStdout = 0;

  fprintf(stderr, "SPKtools SPKx 0.01\n");
  fprintf(stderr, "Copyright (c) 2013 René 'rsoft' Ilgner\n");
  fprintf(stderr, "Licensed under GPLv3. See LICENSE for further details.\n\n");

  if(argc < 3) {
    fprintf(stderr, "Syntax: %s <archive> <virtual file> [out file]\n", argv[0]);
    fprintf(stderr, "SPKx extracts a virtual file from an SPK archive and stores it\n");
    fprintf(stderr, "using either the given 'out file' name or the name the virtual\n");
    fprintf(stderr, "file is called.\n");
    fprintf(stderr, "If the out file name is -, the contents get written to STDOUT.\n");
    return EXIT_FAILURE;
  }

  spkInit(&state);

  if(spkOpenArchive(&state, argv[1])) {
    fprintf(stderr, "Archive broken or non-existant.\n");
    return EXIT_FAILURE;
  }

  readFile = spkFopen(&state, argv[2]);

  if(NULL == readFile) {
    fprintf(stderr, "Error opening virtual file!\n");
    return EXIT_FAILURE;
  }

  if(argc > 3) {
    if(argv[3][0] == '-') {
      writeFile = stdout;
      useStdout = 1;
    }
  }

  if(!useStdout) {
    if(argc > 3) {
      writeFile = fopen(argv[3], "wb");
    } else {
      writeFile = fopen(argv[2], "wb");
    }

    if(NULL == writeFile) {
      fprintf(stderr, "Error opening out file!\n");
      return EXIT_FAILURE;
    }
  }

  unsigned int readCount;

  for(;;) { // A quicker replacement for "while(1)"
    readCount = spkFread(readBuffer, 1, sizeof(readBuffer), readFile);

    if(!readCount) break;

    fwrite(readBuffer, 1, readCount, writeFile);
  }

  if(!useStdout)
    fclose(writeFile);

  if(spkFclose(readFile)) {
    fprintf(stderr, "Error closing virtual file!\n");
    return EXIT_FAILURE;
  }

  if(spkCloseArchive(&state)) {
    fprintf(stderr, "Error closing archive!\n");
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
