#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

/* Function to perform encryption based on given key and plaintext */
void performEncryption(char *plaintext, char *key, char *ciphertext) {
    int plaintext_length = strlen(plaintext);
    int key_length = strlen(key);

    for (int i = 0; i < plaintext_length; i++) {
        char plaintext_char = plaintext[i];
        char key_char = key[i % key_length];                                    // Repeating key

        if (plaintext_char == ' ') {
            ciphertext[i] = ' ';
        } else {
            int plaintext_val = plaintext_char - 'A';                           // Convert to 0-25 range
            int key_val = key_char - 'A';                                       
            int encrypted_val = (plaintext_val + key_val) % 27;
            ciphertext[i] = encrypted_val + 'A';                                // Convert back to ASCII range
        }
    }
    ciphertext[plaintext_length] = '\0';                                        // Null-terminate the ciphertext
}


int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s listening_port\n", argv[0]);
        return 1;
    }

    int listening_port = atoi(argv[1]);                                             // Create socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("Error creating socket");
        return 1;
    }

    struct sockaddr_in server_addr;                                                 // Set up sockaddr_in structure
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(listening_port);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {     // Bind socket to the specified port
        perror("Error binding");
        return 1;
    }

    if (listen(sockfd, 5) == -1) {                                                      // Listen for incoming connections
        perror("Error listening");
        return 1;
    }

    while (1) {                                                                         // Accept connections and serve clients
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int client_sockfd = accept(sockfd, (struct sockaddr *)&client_addr, &client_len);
        if (client_sockfd == -1) {
            perror("Error accepting connection");
            continue;
        }

        pid_t child_pid = fork();                                                       // Fork a child process to handle the client
        if (child_pid == -1) {
            perror("Error forking");
            close(client_sockfd);
            continue;
        }

        if (child_pid == 0) { 
            close(sockfd);
            char plaintext[512];
            char key[512];
            ssize_t plaintext_len = recv(client_sockfd, plaintext, sizeof(plaintext), 0);
            ssize_t key_len = recv(client_sockfd, key, sizeof(key), 0);
            if (plaintext_len == -1 || key_len == -1) {
                perror("Error receiving data");
                close(client_sockfd);
                exit(1);
            }

            char ciphertext[512];
            performEncryption(plaintext, key, ciphertext);

            ssize_t sent_bytes = send(client_sockfd, ciphertext, strlen(ciphertext), 0);
            if (sent_bytes == -1) {
                perror("Error sending data");
                close(client_sockfd);
                exit(1);
            }
            close(client_sockfd);
            exit(0);
        } else { 
            close(client_sockfd);
        }
    }
    close(sockfd);
    return 0;
}
