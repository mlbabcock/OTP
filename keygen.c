#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char *argv[]) {
  if (argc != 2) {
    fprintf(stderr, "Usage: keygen keylength\n");
    exit(1);
  }

  int key_length = atoi(argv[1]);
  char key[key_length + 1];
  for (int i = 0; i < key_length; i++) {
    key[i] = 65 + rand() % 27;
  }
  key[key_length] = '\n';
  printf("%s", key);
  return 0;
}
