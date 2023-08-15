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
  char plaintext[1024];
  int plaintext_length;
  char key[1024];
  int key_length;
  char ciphertext[1024];
  int bytes_sent;
  int bytes_received;

  struct sockaddr_in address;

  printf("Enter the plaintext: ");
  fgets(plaintext, sizeof(plaintext), stdin);
  plaintext_length = strlen(plaintext);

  printf("Enter the key: ");
  fgets(key, sizeof(key), stdin);
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

  bytes_received = recv(socket_fd, ciphertext, sizeof(ciphertext), 0);
  if (bytes_received == -1) {
    perror("recv");
    exit(1);
  }

  close(socket_fd);

  printf("The ciphertext is: ");
  fwrite(ciphertext, 1, bytes_received, stdout);

  return 0;
}
