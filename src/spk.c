#include <string.h>

#include "spk.h"

char spkMagic[6] = {'S', 'i', 'm', 'P', 'a', 'K'};

/*
 * spkInit - Initialize (clear) a state data structure
 * Returns void
 * Takes a pointer to a state struct (struct spkState*)
 */
void spkInit(struct spkState* state) {
  state->archiveHandle = NULL;
  state->fileCount = 0;

  spkCloseAllFiles(state);
}


int spkSetArchive(struct spkState* state, FILE* fh) {
  /*
   * If the file handle given to us is a null pointer, return 1 and don't modify the state
   */
  if(!fh)
    return 1;

  state->archiveHandle = fh;

  return 0;
}

int spkCloseAllFiles(struct spkState* state) {
  int i, err = 0;

  /*
   * Try closing all files and count the already closed files
   */
  for(i = 0; i < MAX_OPEN_FILES; i++) {
    err += spkFclose(&state->handles[i]);
  }

  return err;
}

int spkUnsetArchvie(struct spkState* state) {
  /*
   * Check if the archive handle is already a null pointer and return an error if that is the case
   */
  if(!state->archiveHandle)
    return 1;

  state->archiveHandle = NULL;

  return 0;
}


int spkOpenArchive(struct spkState* state, char* filename) {
  if(spkSetArchive(state, fopen(filename, "rb")))
    return 1;

  if(spkLoadHeader(state))
    return 1;

  return spkGenLUT(state);
}

int spkCloseArchive(struct spkState* state) {
  /*
   * First off, drop all file handles
   */

  spkCloseAllFiles(state);

  /*
   * Next, reset the file count and close the archive handle.
   */
  
  state->fileCount = 0;
  if(state->archiveHandle)
    return fclose(state->archiveHandle);

  return 1;
}


struct spkFileHandle* spkFopen(struct spkState* state, char* filename) {
  int i, index = -1;
  
  /*
   * Search for a file with the specified name
   */
  for(i = 0; i < state->fileCount; i++) {
    if(spkReadChunk(state, i + 1)) // Load the current chunk into memory
      return NULL;

    struct FATentry* currEntry = (struct FATentry*) state->entryCache;

    if(currEntry->fileName[ENTRYSIZE-3] != 0)
      return NULL;

    if(0 == strncmp(filename, currEntry->fileName, ENTRYSIZE - 3)) {
      index = i;
      break;
    }
  }

  /*
   * If there is no free space, return a NULL pointer
   */
  if(-1 == index)
    return NULL;

  return spkFopenByIndex(state, index);
}

struct spkFileHandle* spkFopenByIndex(struct spkState* state, uint8_t index) {
  int i, handleIndex = -1;

  /*
   * Search for a free file handle
   */
  for(i = 0; i < MAX_OPEN_FILES; i++) {
    if(NULL == state->handles[i].state) {
      handleIndex = i;
      break;
    }
  }

  /*
   * If there is no free space, return a NULL pointer
   */
  if(-1 == handleIndex)
    return NULL;

  state->handles[handleIndex].state = state;
  state->handles[handleIndex].fileIndex = index;
  state->handles[handleIndex].pos = 0;

  return &state->handles[handleIndex];
}

int spkFclose(struct spkFileHandle* spkFh) {
  /* Return an error value if the file handle is already closed */
  if(NULL == spkFh->state)
    return 1;

  spkFh->state = NULL;
  return 0;
}

size_t spkFread(void* dest, size_t size, size_t count, struct spkFileHandle* spkFh) {
  uint16_t fileSize, pos;
  FILE* archiveHandle;
  size_t fread_return;

  if((0 == size) || (0 == count))
    return 0;

  fileSize = spkFh->state->LookUpTable[(uint8_t)spkFh->fileIndex].fileSize;
  archiveHandle = spkFh->state->archiveHandle;
  pos = spkFh->pos;

  if((size * count) > (fileSize - pos))
    count = (fileSize - pos) / size;

  spkFseek(spkFh, pos, SEEK_SET);

  fread_return = fread(dest, size, count, archiveHandle);

  spkFh->pos += fread_return * size;

  return fread_return;
}

int spkFseek(struct spkFileHandle* spkFh, long offset, int whence) {
  uint8_t fileIndex = spkFh->fileIndex;
  uint16_t pos = spkFh->pos;
  struct LUTentry* entry = &spkFh->state->LookUpTable[(uint8_t)fileIndex];
  uint16_t fileSize = entry->fileSize;
  uint32_t absoluteOffset = entry->absoluteOffset;

  switch(whence) {
    case SEEK_SET: {
      if(offset > fileSize || offset < 0)
        return -1;

      pos = offset;
      spkFh->pos = pos;
      return fseek(spkFh->state->archiveHandle, absoluteOffset + pos, SEEK_SET);
      break;
    }
    case SEEK_END: {
      if(((fileSize - offset) < 0) || (fileSize - offset) > fileSize)
        return -1;

      pos = fileSize - offset;
      spkFh->pos = pos;
      return fseek(spkFh->state->archiveHandle, absoluteOffset + pos, SEEK_SET);
      break;
    }
    case SEEK_CUR: {
      if(((pos + offset) < 0) || (pos + offset) > fileSize)
        return -1;

      pos += offset;
      spkFh->pos = pos;
      return fseek(spkFh->state->archiveHandle, absoluteOffset + pos, SEEK_SET);
      break;
    }
    default: {
      return -1;
    }
  }

  return -1;
}

long spkFtell(struct spkFileHandle* spkFh) {
  return spkFh->pos;
}


int spkLoadHeader(struct spkState* state) {
  if(spkReadChunk(state, 0))
    return 1;

  struct spkHeader* header = (struct spkHeader*) state->entryCache;

  /* Check the magic */
  if(strncmp(header->magic, spkMagic, sizeof(spkMagic)) != 0)
    return 1;

  if(header->version != SPK_VERSION)
    return 1;

  state->fileCount = header->fileCount;

  return 0;
}

int spkGenLUT(struct spkState* state) {
  int i;
  uint32_t totalSize = (state->fileCount + 1) * ENTRYSIZE;;

  for(i = 0; i < state->fileCount; i++) {
    if(spkReadChunk(state, i + 1))
      return 1;

    struct FATentry* currEntry = (struct FATentry*) state->entryCache;

    state->LookUpTable[i].fileSize = currEntry->sizeLSB | (currEntry->sizeMSB << 8);
    state->LookUpTable[i].absoluteOffset = totalSize;
    totalSize += state->LookUpTable[i].fileSize;
  }
  return 0;
}


/*
 * Reads a chunk to the entry cache
 */
int spkReadChunk(struct spkState* state, uint8_t index) {
  if(-1 == fseek(state->archiveHandle, ENTRYSIZE * index, SEEK_SET))
    return 1;

  if(fread(state->entryCache, ENTRYSIZE, 1, state->archiveHandle) < 1)
    return 1;

  return 0;
}
