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
  if (argc != 2) {
    fprintf(stderr, "Usage: enc_server listening_port\n");
    exit(1);
  }

  int listening_port = atoi(argv[1]);
  int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (socket_fd == -1) {
    perror("socket");
    exit(1);
  }

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

  listen(socket_fd, MAX_CONNECTIONS);

  while (1) {
    int client_fd = accept(socket_fd, NULL, NULL);
    if (client_fd == -1) {
      perror("accept");
      continue;
    }

    pid_t pid = fork();
    if (pid == 0) {
      close(socket_fd); 
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

      char ciphertext[1024];
      for (int i = 0; i < bytes_received; i++) {
        ciphertext[i] = plaintext[i] ^ key[i];
      }

      bytes_sent = send(client_fd, ciphertext, sizeof(ciphertext), 0);
      if (bytes_sent == -1) {
        perror("send");
        exit(1);
      }

      close(client_fd);
      exit(0);
    } else if (pid < 0) {
      perror("fork");
      continue;
    }
  }
  return 0;
}
