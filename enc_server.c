#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define MAX_CONNECTIONS 5

int main(int argc, char *argv[]) {
  // Check for the correct number of arguments
  if (argc != 2) {
    fprintf(stderr, "Usage: enc_server listening_port\n");
    exit(1);
  }

  // Get the listening port
  int listening_port = atoi(argv[1]);

  // Create a socket
  int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (socket_fd == -1) {
    perror("socket");
    exit(1);
  }

  // Bind the socket to the listening port
  struct sockaddr_in address;
  memset(&address, 0, sizeof(address));
  address.sin_family = AF_INET;
  address.sin_port = htons(listening_port);
  address.sin_addr.s_addr = INADDR_ANY;
  int bind_result = bind(socket_fd, (struct sockaddr *)&address, sizeof(address));
  if (bind_result == -1) {
    perror("bind");
    exit(1);
  }

  // Listen for connections
  listen(socket_fd, MAX_CONNECTIONS);

  while (1) {
    // Accept a connection
    int client_fd = accept(socket_fd, NULL, NULL);
    if (client_fd == -1) {
      perror("accept");
      continue;
    }

    // Create a child process to handle the connection
    pid_t pid = fork();
    if (pid == 0) {
      // This is the child process
      close(socket_fd); // Close the listening socket

      // Receive the plaintext and key from the client
      char plaintext[1024];
      char key[1024];
      int bytes_received = recv(client_fd, plaintext, sizeof(plaintext), 0);
      if (bytes_received == -1) {
        perror("recv");
        exit(1);
      }
      bytes_received = recv(client_fd, key, sizeof(key), 0);
      if (bytes_received == -1) {
        perror("recv");
        exit(1);
      }

      // Encrypt the plaintext with the key
      char ciphertext[1024];
      for (int i = 0; i < bytes_received; i++) {
        ciphertext[i] = plaintext[i] ^ key[i];
      }

      // Send the ciphertext back to the client
      bytes_sent = send(client_fd, ciphertext, sizeof(ciphertext), 0);
      if (bytes_sent == -1) {
        perror("send");
        exit(1);
      }

      // Close the connection
      close(client_fd);
      exit(0);
    } else if (pid < 0) {
      perror("fork");
      continue;
    }
  }

  return 0;
}
