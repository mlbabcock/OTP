#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

char key_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ ";

// Function to generate a random key of a specified length
void generate_key(int length, char *str)
{
    for (int i = 0; i < length; ++i)
    {
        // Generate a random index within the key_chars array and assign the character at that index to the key
        str[i] = key_chars[rand() % strlen(key_chars)];
    }
}

int main(int argc, char *argv[])
{
    // Check if the correct number of command-line arguments is provided
    if (argc < 2)
    {
        fprintf(stderr, "USAGE: %s keylength\n", argv[0]);
        return EXIT_FAILURE;
    }

    srand((unsigned int)time(NULL));                                // Seed the random number generator using the current time
    int key_length = atoi(argv[1]);                                 // Convert the second command-line argument to an integer (key length)

    // Allocate memory for the key string, and initialize it with null characters
    char *key_string = calloc(key_length + 1, sizeof(char));
    memset(key_string, '\0', key_length + 1);
    generate_key(key_length, key_string);                           // Generate a random key of the specified length

    fprintf(stdout, "%s\n", key_string);                            // Print the generated key to the standard output
    fflush(stdout);                                                 // Free the dynamically allocated memory for the key string

    return EXIT_SUCCESS;
}
