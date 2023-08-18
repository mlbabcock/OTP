#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

// The characters used for generating the key
char key_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ ";

int main(int argc, char *argv[]) {
  // Check for correct number of command-line arguments
  if (argc < 2) {
    fprintf(stderr, "Usage: %s keygen keylength\n", argv[0]);
    return EXIT_FAILURE;
  }

  // Seed the random number generator using current time
  srand((unsigned int)time(NULL));

  // Parse the desired key length from command-line argument
  int key_length = atoi(argv[1]);

  // Allocate memory for the key string and initialize to null characters
  char *key_string = calloc(key_length + 1, sizeof(char));
  memset(key_string, '\0', key_length + 1);

  // Generate the random key string
  for (int i = 0; i < key_length; i++) {
    key_string[i] = key_chars[rand() % strlen(key_chars)]; // Choose a random character from key_chars
  }

  // Print the generated key to standard output
  fprintf(stdout, "%s\n", key_string);

  // Flush the standard output to ensure the message is sent
  fflush(stdout);

  // Clean up allocated memory and exit
  free(key_string);
  return EXIT_SUCCESS;
}
