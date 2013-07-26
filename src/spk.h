#ifndef __SPK_H_
#define __SPK_H_

#ifdef PSXGCC
#warning "Compiling with PSXSDK, using dodgy stdint.h replacement that might explode in your face!"
#include "../include/stdint.h"
#else
#include <stdint.h>
#endif

#include <stdio.h>
#include <stddef.h>

#define ENTRYSIZE 64
#define MAX_OPEN_FILES 16

#define SPK_VERSION 0x00

struct spkHeader {
  char magic[6];
  uint8_t version;
  uint8_t fileCount;
  uint8_t padding[ENTRYSIZE-8];
} __attribute__((packed));

struct FATentry {
  uint8_t sizeLSB;
  uint8_t sizeMSB;
  char fileName[ENTRYSIZE-2];
} __attribute__((packed));

struct LUTentry {
  uint16_t fileSize;
  uint32_t absoluteOffset; // Absolute offset in the file. Invalid if 0x00000000
};

struct spkFileHandle {
  struct spkState* state; // Pointer to the state, fh closed if NULL
  uint8_t fileIndex;
  uint16_t pos;
};

struct spkState {
  FILE* archiveHandle;
  uint8_t fileCount;
  uint8_t entryCache[ENTRYSIZE];
  struct LUTentry LookUpTable[255];
  struct spkFileHandle handles[MAX_OPEN_FILES];
};

void spkInit(struct spkState* state);

int spkSetArchive(struct spkState* state, FILE* fh);
int spkCloseAllFiles(struct spkState* state);
int spkUnsetArchvie(struct spkState* state);

int spkOpenArchive(struct spkState* state, char* filename);
int spkCloseArchive(struct spkState* state);

struct spkFileHandle* spkFopen(struct spkState* state, char* filename);
struct spkFileHandle* spkFopenByIndex(struct spkState* state, uint8_t index);
int spkFclose(struct spkFileHandle* spkFh);

size_t spkFread(void* dest, size_t size, size_t count, struct spkFileHandle* spkFh);
int spkFseek(struct spkFileHandle* spkFh, long offset, int whence);
long spkFtell(struct spkFileHandle* spkFh);

int spkLoadHeader(struct spkState* state);
int spkGenLUT(struct spkState* state);

int spkReadChunk(struct spkState* state, uint8_t index);

#endif /* !__SPK_H_ */
