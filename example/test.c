#include <stdio.h>
#include <stdlib.h>

#include "libspk.h"

#ifndef EXIT_FAILURE
#define EXIT_FAILURE 1
#endif

#ifndef EXIT_SUCCESS
#define EXIT_SUCCESS 0
#endif

int main() {
  struct spkState state;
  char testFileContents[42] = {0};

  spkInit(&state);

  if(spkOpenArchive(&state, "lol.spk")) {
    printf("Looks like the archive has been... spiked.\n");
    return EXIT_FAILURE;
  }

  struct spkFileHandle* test = spkFopen(&state, "This is a file!");

  if(NULL == test) {
    printf("Error opening file!\n");
    return EXIT_FAILURE;
  }

  printf("Current position in file: %ld\n", spkFtell(test));
  printf("Read %zu characters\n", spkFread(testFileContents, 1, 10, test));
  printf("Contents: '%s'\n", testFileContents);
  printf("Current position in file: %ld\n", spkFtell(test));
  printf("Read %zu characters\n", spkFread(testFileContents, 1, 10, test));
  printf("Current position in file: %ld\n", spkFtell(test));

  if(spkFclose(test)) {
    printf("Error closing file!\n");
    return EXIT_FAILURE;
  }

  if(spkCloseArchive(&state)) {
    printf("Error closing archive!\n");
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
