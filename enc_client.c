#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s plaintext key port\n", argv[0]);
        return 1;
    }

    char *plaintext_filename = argv[1];
    char *key_filename = argv[2];
    int port = atoi(argv[3]);

    FILE *plaintext_file = fopen(plaintext_filename, "r");
    if (!plaintext_file) {
        perror("Error opening plaintext file");
        return 1;
    }
    char plaintext[512];
    if (fgets(plaintext, sizeof(plaintext), plaintext_file) == NULL) {
        perror("Error reading plaintext");
        fclose(plaintext_file);
        return 1;
    }
    fclose(plaintext_file);

    FILE *key_file = fopen(key_filename, "r");
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
    fclose(key_file);
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("Error creating socket");
        return 1;
    }

    struct hostent *server = gethostbyname("localhost");
    if (server == NULL) {
        perror("Error resolving hostname");
        return 1;
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    memcpy(&server_addr.sin_addr.s_addr, server->h_addr, server->h_length);

    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {          // Connect to the server
        perror("Error connecting to server");
        return 2;
    }

    if (send(sockfd, plaintext, strlen(plaintext), 0) == -1 ||                                  // Send plaintext and key to server
        send(sockfd, key, strlen(key), 0) == -1) {
        perror("Error sending data to server");
        close(sockfd);
        return 1;
    }
    char ciphertext[512];                                                                       // Receive ciphertext from server
    ssize_t ciphertext_len = recv(sockfd, ciphertext, sizeof(ciphertext), 0);
    if (ciphertext_len == -1) {
        perror("Error receiving ciphertext");
        close(sockfd);
        return 1;
    }
    fwrite(ciphertext, sizeof(char), ciphertext_len, stdout);
    close(sockfd);
    return 0;
}

