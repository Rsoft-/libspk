#ifndef __LIBSPK_H_
#define __LIBSPK_H_

#ifdef PSXGCC
#warning "Compiling with PSXSDK, using dodgy stdint.h replacement that might explode in your face!"
#include "stdint.h"
#else
#include <stdint.h>
#endif

#include <stdio.h>
#include <stddef.h>

#define ENTRYSIZE 64
#define MAX_OPEN_FILES 16

#define SPK_VERSION 0x00

struct LUTentry {
  uint16_t fileSize;
  uint32_t absoluteOffset; // Absolute offset in the file. Invalid if 0x00000000
};

struct spkFileHandle {
  struct spkState* state; // Pointer to the state, fh closed if NULL
  char fileIndex;
  uint16_t pos;
};

struct spkState {
  FILE* archiveHandle;
  char fileCount;
  char entryCache[ENTRYSIZE];
  struct LUTentry LookUpTable[255];
  struct spkFileHandle handles[MAX_OPEN_FILES];
};

void spkInit(struct spkState* state);

int spkOpenArchive(struct spkState* state, char* filename);
int spkCloseArchive(struct spkState* state);

struct spkFileHandle* spkFopen(struct spkState* state, char* filename);
int spkFclose(struct spkFileHandle* spkFh);

size_t spkFread(void* dest, size_t size, size_t count, struct spkFileHandle* spkFh);
int spkFseek(struct spkFileHandle* spkFh, long offset, int whence);
long spkFtell(struct spkFileHandle* spkFh);

#endif /* !__LIBSPK_H_ */
