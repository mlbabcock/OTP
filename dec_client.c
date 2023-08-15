#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdbool.h>
#include <fcntl.h>
#include <ctype.h>

#define MAX 9000000

void error(const char *message){
    perror(message);
    exit(1);
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s ciphertext key port\n", argv[0]);
        return 1;
    }

    char *ciphertext_filename = argv[1];
    char *key_filename = argv[2];
    int port = atoi(argv[3]);

    FILE *ciphertext_file = fopen(ciphertext_filename, "r");                                // Read ciphertext from file
    if (!ciphertext_file) {
        perror("Error opening ciphertext file");
        return 1;
    }
    char ciphertext[512];
    if (fgets(ciphertext, sizeof(ciphertext), ciphertext_file) == NULL) {
        perror("Error reading ciphertext");
        fclose(ciphertext_file);
        return 1;
    }

    // Remove newline characters at the end of the ciphertext if present
    size_t ciphertext_length = strlen(ciphertext);
    if (ciphertext_length > 0 && ciphertext[ciphertext_length - 1] == '\n') {
        ciphertext[ciphertext_length - 1] = '\0';
    }
    fclose(ciphertext_file);

    FILE *key_file = fopen(key_filename, "r");                                              // Read key from file
    if (!key_file) {    
        perror("Error opening key file");
        return 1;
    }
    char key[512];
    if (fgets(key, sizeof(key), key_file) == NULL) {
        perror("Error reading key");
        fclose(key_file);
        return 1;
    }

    // Remove newline characters at the end of the key if present
    size_t key_length = strlen(key);
    if (key_length > 0 && key[key_length - 1] == '\n') {
        key[key_length - 1] = '\0';
    }
    fclose(key_file);

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);                                             // Create socket
    if (sockfd == -1) {
        perror("Error creating socket");
        return 1;
    }

    struct hostent *server = gethostbyname("localhost");
    if (server == NULL) {
        perror("Error resolving hostname");
        return 1;
    }

    /* Set up sockaddr_in structure */
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    memcpy(&server_addr.sin_addr.s_addr, server->h_addr, server->h_length);

    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {       // Connect to the server
        perror("Error connecting to server");
        return 2;
    }

    if (send(sockfd, ciphertext, strlen(ciphertext), 0) == -1 ||                           // Send ciphertext and key to server
        send(sockfd, key, strlen(key), 0) == -1) {
        perror("Error sending data to server");
        close(sockfd);
        return 1;
    }

    char plaintext[512];                                                                   // Receive plaintext from server
    ssize_t plaintext_len = recv(sockfd, plaintext, sizeof(plaintext), 0);
    if (plaintext_len == -1) {
        perror("Error receiving plaintext");
        close(sockfd);
        return 1;
    }

    fwrite(plaintext, sizeof(char), plaintext_len, stdout);
    close(sockfd);
    return 0;
}
