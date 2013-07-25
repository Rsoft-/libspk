/*
 * SPKtools genSPK - Horribly hacked-together SPK generator...
 * MOAR Mate!
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>

#include "../src/spk.h"

void assemblePath(char* out, char* path, char* fileName) {
  strcpy(out, path);
  strcat(out, "/");
  strcat(out, fileName);
}

int main(int argc, char** argv) {
  FILE* spkFile;

  DIR* dir;
  struct dirent* ent;

  FILE* fh;

  struct FATentry* entries = NULL;

  printf("SPKtools genSPK 0.01\n");

  if(argc < 3) {
    printf("Syntax: %s <OUTFILE> <INDIR>\n", argv[0]);
    return EXIT_FAILURE;
  }

  spkFile = fopen(argv[1], "wb");
  if(!spkFile) {
    printf("Error opening spk file.\n");
    return EXIT_FAILURE;
  }

  dir = opendir(argv[2]);
  if(!dir) {
    printf("Error opening directory.\n");
    fclose(spkFile);
    return EXIT_FAILURE;
  }

  char* filePath = malloc(strlen(argv[2]) + 1 + ENTRYSIZE - 2);

  if(filePath == NULL) {
    printf("Out of memory!\n");
    return EXIT_FAILURE;
  }

  if((entries = malloc(sizeof(struct FATentry))) == NULL) {
    printf("Out of memory!\n");
    return EXIT_FAILURE;
  }

  printf("Indexing...\n");

  int i = 0;

  while(ent = readdir(dir)) {
    i++;
    if(i > 255) {
      printf("Error: Maximum file count (255) reached!\n");
      return EXIT_FAILURE;
    }

    if(((ent->d_name[0] == '.') && (ent->d_name[1] == 0)) ||
       ((ent->d_name[0] == '.') && (ent->d_name[1] == '.') && (ent->d_name[2] == 0))) {
      //printf("Skipping %s\n", ent->d_name);
      i--;
      continue;
    }

    printf("%s\n", ent->d_name);
    if(strnlen(ent->d_name, ENTRYSIZE-2) > ENTRYSIZE-3) {
      printf("Error processing '%s': Name longer than %d characters!\n", ent->d_name, ENTRYSIZE-3);
      return EXIT_FAILURE;
    }

    assemblePath(filePath, argv[2], ent->d_name);

    if((fh = fopen(filePath, "rb")) == NULL) {
      printf("Error opening '%s'!\n", filePath);
      return EXIT_FAILURE;
    }

    fseek(fh, 0, SEEK_END);

    long fileSize = ftell(fh);
    fclose(fh);
    printf("  Size: %d\n", fileSize);

    if(fileSize > 0xFFFF) {
      printf("Error: File too big!\n");
      return EXIT_FAILURE;
    }

    entries = realloc(entries, sizeof(struct FATentry)*i);
    if(!entries) {
      printf("Out of memory!\n");
      return EXIT_FAILURE;
    }

    // Zero out the memory
    memset(&entries[i-1], 0, sizeof(struct FATentry));

    strcpy(entries[i-1].fileName, ent->d_name);

    entries[i-1].sizeLSB = fileSize & 0xFF;
    entries[i-1].sizeMSB = (fileSize & 0xFF00) >> 8;
  }

  printf("Writing package...\n");

  struct spkHeader header = {0};
  header.magic[0] = 'S';
  header.magic[1] = 'i';
  header.magic[2] = 'm';
  header.magic[3] = 'P';
  header.magic[4] = 'a';
  header.magic[5] = 'K';
  header.version = 0;
  header.fileCount = i;

  if(fwrite(&header, sizeof(header), 1, spkFile) != 1) {
    printf("Error writing header!\n");
    return EXIT_FAILURE;
  }

  if(fwrite(entries, sizeof(struct FATentry), i, spkFile) != i) {
    printf("Error writing FAT!\n");
    return EXIT_FAILURE;
  }

  int j;
  for(j = 0; j < (i-1); j++) {
    assemblePath(filePath, argv[2], entries[j].fileName);
    fh = fopen(filePath, "rb");
    if(fh == NULL) {
      printf("Error opening file '%s'!\nreal Name: '%s'\n", filePath, entries[j].fileName);
      return EXIT_FAILURE;
    }

    char buffer[1024];
    int read;

    while(read = fread(buffer, 1, 1024, fh)) {
      if(fwrite(buffer, 1, read, spkFile) != read) {
        printf("Error writing file!\n");
	return EXIT_FAILURE;
      }
    }
    fclose(fh);
  }

  fclose(spkFile);

  return EXIT_SUCCESS;
}
