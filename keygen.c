#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Function to generate a random character from the allowed set */
char getRandomCharacter() {
    const char allowed_characters[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ ";
    int num_characters = sizeof(allowed_characters) - 1; // Exclude null terminator
    return allowed_characters[rand() % num_characters];
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s keylength\n", argv[0]);
        return 1;
    }

    int key_length = atoi(argv[1]);
    srand(time(NULL));  // Seed the random number generator

    for (int i = 0; i < key_length; i++) {  // Generate and output the key
        char random_char = getRandomCharacter();
        putchar(random_char);
    }

    putchar('\n');  // Output a newline at the end
    return 0;
}
