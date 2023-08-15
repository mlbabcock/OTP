#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

/* Function to perform decryption based on given key and ciphertext */
void performDecryption(char *ciphertext, char *key, char *plaintext) {
    int ciphertext_length = strlen(ciphertext);
    int key_length = strlen(key);

    for (int i = 0; i < ciphertext_length; i++) {
        char ciphertext_char = ciphertext[i];
        char key_char = key[i % key_length];                                           // Repeating key

        if (ciphertext_char == ' ') {
            plaintext[i] = ' ';
        } else {
            int ciphertext_val = ciphertext_char - 'A'; 
            int key_val = key_char - 'A';                                               // Convert to 0-25 range
            int decrypted_val = (ciphertext_val - key_val) % 27;
            
            if (decrypted_val < 0) {                                                    // Handle negative values due to modulo
                decrypted_val += 27;
            }
            plaintext[i] = decrypted_val + 'A';                                         // Convert back to ASCII range
        }
    }
    plaintext[ciphertext_length] = '\0'; 
}


int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s listening_port\n", argv[0]);
        return 1;
    }

    int listening_port = atoi(argv[1]);

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);                                           // Create socket
    if (sockfd == -1) {
        perror("Error creating socket");
        return 1;
    }

    struct sockaddr_in server_addr;                                                        // Set up sockaddr_in structure
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(listening_port);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {        // Bind socket to the specified port
        perror("Error binding");
        return 1;
    }

    if (listen(sockfd, 5) == -1) {                                                        // Listen for incoming connections
        perror("Error listening");
        return 1;
    }

    while (1) {                                                                             // Accept connections and serve clients
        struct sockaddr_in client_addr; 
        socklen_t client_len = sizeof(client_addr);
        int client_sockfd = accept(sockfd, (struct sockaddr *)&client_addr, &client_len);
        if (client_sockfd == -1) {
            perror("Error accepting connection");
            continue;
        }

        pid_t child_pid = fork();
        if (child_pid == -1) {
            perror("Error forking");
            close(client_sockfd);
            continue;
        }

        if (child_pid == 0) { 
            close(sockfd);                                                                 // Read ciphertext and key from the client
            char ciphertext[512];
            char key[512];
            ssize_t ciphertext_len = recv(client_sockfd, ciphertext, sizeof(ciphertext), 0);
            ssize_t key_len = recv(client_sockfd, key, sizeof(key), 0);
            if (ciphertext_len == -1 || key_len == -1) {
                perror("Error receiving data");
                close(client_sockfd);
                exit(1);
            }

            char plaintext[512];
            performDecryption(ciphertext, key, plaintext);
            ssize_t sent_bytes = send(client_sockfd, plaintext, strlen(plaintext), 0);
            if (sent_bytes == -1) {
                perror("Error sending data");
                close(client_sockfd);
                exit(1);
            }
            close(client_sockfd);                                                     // Close the client socket and exit the child process
            exit(0);
        } else { 
            close(client_sockfd);
        }
    }
    close(sockfd);
    return 0;
}

