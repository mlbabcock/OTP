#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

char key_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ ";

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Usage: %s keygen keylength\n", argv[0]);
    return EXIT_FAILURE;
  }
  srand((unsigned int)time(NULL));
  int key_length = atoi(argv[1]);
  char *key_string = calloc(key_length + 1, sizeof(char));
  memset(key_string, '\0', key_length + 1);

  for (int i = 0; i < key_length; i++) {
    key_string[i] = key_chars[rand() % strlen(key_chars)];
  }

  fprintf(stdout, "%s\n", key_string);
  fflush(stdout);
  return EXIT_SUCCESS; 
}
