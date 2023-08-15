#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/socket.h>
#include <errno.h>

int main() {
  int socket_fd;
  char *plaintext;
  int plaintext_length;
  char *key;
  int key_length;
  char *ciphertext;
  int bytes_sent;
  int bytes_received;

  struct sockaddr_in address;

  plaintext = malloc(1024 * sizeof(char));
  printf("Enter the plaintext: ");
  fgets(plaintext, 1024, stdin);
  plaintext_length = strlen(plaintext);

  key = malloc(1024 * sizeof(char));
  printf("Enter the key: ");
  fgets(key, 1024, stdin);
  key_length = strlen(key);
  
  socket_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (socket_fd == -1) {
    perror("socket");
    exit(1);
  }

  address.sin_family = AF_INET;
  address.sin_port = htons(57171);
  address.sin_addr.s_addr = INADDR_ANY;
  int connect_result = connect(socket_fd, (struct sockaddr *)&address, sizeof(address));
  if (connect_result == -1) {
    perror("connect");
    exit(1);
  }
  bytes_sent = send(socket_fd, plaintext, plaintext_length, 0);
  if (bytes_sent == -1) {
    perror("send");
    exit(1);
  }
  bytes_sent = send(socket_fd, key, key_length, 0);
  if (bytes_sent == -1) {
    perror("send");
    exit(1);
  }
  ciphertext = malloc(1024 * sizeof(char));
  bytes_received = recv(socket_fd, ciphertext, 1024, 0);
  if (bytes_received == -1) {
    perror("recv");
    exit(1);
  }
  close(socket_fd);
  printf("The ciphertext is: %s\n", ciphertext);

  free(plaintext);
  free(key);
  free(ciphertext);

  return 0;
}
