#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

int main(int argc, char *argv[]) {
  if (argc != 3) {
    fprintf(stderr, "Usage: enc_client plaintext key port\n");
    exit(1);
  }

  char *plaintext_file_name = argv[1];
  char *key_file_name = argv[2];
  int port_number = atoi(argv[3]);
  FILE *plaintext_file = fopen(plaintext_file_name, "r");
  if (plaintext_file == NULL) {
    fprintf(stderr, "Could not open plaintext file: %s\n", plaintext_file_name);
    exit(1);
  }

  FILE *key_file = fopen(key_file_name, "r");
  if (key_file == NULL) {
    fprintf(stderr, "Could not open key file: %s\n", key_file_name);
    exit(1);
  }

  char plaintext[1024];
  int plaintext_length = fread(plaintext, 1, sizeof(plaintext), plaintext_file);
  if (plaintext_length == -1) {
    perror("fread");
    exit(1);
  }
  plaintext_file.close();

  char key[1024];
  int key_length = fread(key, 1, sizeof(key), key_file);
  if (key_length == -1) {
    perror("fread");
    exit(1);
  }
  key_file.close();

  if (key_length < plaintext_length) {
    fprintf(stderr, "Key is too short: %d < %d\n", key_length, plaintext_length);
    exit(1);
  }

  struct sockaddr_in address;
  memset(&address, 0, sizeof(address));
  address.sin_family = AF_INET;
  address.sin_port = htons(port_number);
  address.sin_addr.s_addr = INADDR_ANY;
  int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (socket_fd == -1) {
    perror("socket");
    exit(1);
  }

  int connect_result = connect(socket_fd, (struct sockaddr *)&address, sizeof(address));
  if (connect_result == -1) {
    if (errno == ECONNREFUSED) {
      fprintf(stderr, "Could not connect to enc_server on port %d\n", port_number);
      exit(2);
    } else {
      perror("connect");
      exit(1);
    }
  }

  int bytes_sent = send(socket_fd, plaintext, plaintext_length, 0);
  if (bytes_sent == -1) {
    perror("send");
    exit(1);
  }
  bytes_sent = send(socket_fd, key, key_length, 0);
  if (bytes_sent == -1) {
    perror("send");
    exit(1);
  }

  char ciphertext[1024];
  int bytes_received = recv(socket_fd, ciphertext, sizeof(ciphertext), 0);
  if (bytes_received == -1) {
    perror("recv");
    exit(1);
  }
  close(socket
}
