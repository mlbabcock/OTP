#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/stat.h>

char key_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ ";
char *text_buffer, *key_buffer, *enc_buffer;
int text_size, key_size;

int main(int argc, char *argv[])
{
    int socket_fd, port_number, chars_written, chars_read;
    struct sockaddr_in server_address;

    if (argc < 4)
    {
        fprintf(stderr, "USAGE: %s plaintext keytext port\n", argv[0]);
        exit(1);
    }

    // Check and read text and key files
    struct stat text_stat;
    struct stat key_stat;

    stat(argv[1], &text_stat);
    stat(argv[2], &key_stat);
    text_size = text_stat.st_size;
    key_size = key_stat.st_size;
    FILE *text_file = fopen(argv[1], "r");
    FILE *key_file = fopen(argv[2], "r");

    if (text_file == NULL || key_file == NULL)
    {
        fprintf(stderr, "Error: Cannot open files\n");
        exit(1);
    }

    text_buffer = (char *)calloc(text_size + 1, sizeof(char));
    key_buffer = (char *)calloc(key_size + 1, sizeof(char));
    memset(text_buffer, '\0', text_size);
    memset(key_buffer, '\0', key_size);

    fgets(text_buffer, text_size, text_file);
    fgets(key_buffer, key_size, key_file);
    fclose(text_file);
    fclose(key_file);

    text_size = strlen(text_buffer);
    key_size = strlen(key_buffer);

    // Check characters and sizes
    if (text_size > key_size)
    {
        fprintf(stderr, "Error: key is too short\n");
        exit(1);
    }

    text_buffer[text_size] = '\0';
    key_buffer[key_size] = '\0';

    for (int i = 0; i < text_size; ++i)
    {
        if (strchr(key_chars, text_buffer[i]) == NULL)
        {
            fprintf(stderr, "Error: bad character in text\n");
            exit(1);
        }
    }
    for (int i = 0; i < key_size; ++i)
    {
        if (strchr(key_chars, key_buffer[i]) == NULL)
        {
            fprintf(stderr, "Error: bad character in key\n");
            exit(1);
        }
    }

    // Create socket
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0)
    {
        fprintf(stderr, "CLIENT: Error opening socket\n");
        exit(2);
    }

    // Set up server address
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(atoi(argv[3]));

    struct hostent *hostInfo = gethostbyname("localhost");
    if (hostInfo == NULL)
    {
        fprintf(stderr, "CLIENT: Error, no such host\n");
        exit(2);
    }
    memcpy((char *)&server_address.sin_addr.s_addr, hostInfo->h_addr_list[0], hostInfo->h_length);

    // Connect to server
    if (connect(socket_fd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
    {
        fprintf(stderr, "CLIENT: Error connecting\n");
        exit(2);
    }

    // Initial contact
    int respons;
    char name[] = "enc_client\0";
    if (send(socket_fd, name, strlen(name), 0) < 0)
    {
        fprintf(stderr, "Client: Error sending initial message to server\n");
        exit(2);
    }

    if (recv(socket_fd, &respons, sizeof(respons), 0) < 0)
    {
        fprintf(stderr, "Client: Error receiving initial message from server\n");
        exit(2);
    }

    // Send text and key messages
    if (send(socket_fd, &text_size, sizeof(text_size), 0) < 0 || send(socket_fd, text_buffer, text_size, 0) < 0)
    {
        fprintf(stderr, "Client: Error sending text message to server\n");
        exit(2);
    }

    if (send(socket_fd, &key_size, sizeof(key_size), 0) < 0 || send(socket_fd, key_buffer, key_size, 0) < 0)
    {
        fprintf(stderr, "Client: Error sending key message to server\n");
        exit(2);
    }

    // Receive encrypted message
    int enc_length, total_received = 0, received, index = 0;
    if (recv(socket_fd, &enc_length, sizeof(enc_length), 0) < 0)
    {
        fprintf(stderr, "Client: Error reading from server\n");
        exit(2);
    }
    enc_buffer = (char *)calloc(enc_length + 1, sizeof(char));
    memset(enc_buffer, '\0', enc_length + 1);

    int is_done = 0;
    int data_size;

    while (!is_done)
    {
        if (enc_length - total_received >= 5000)
        {
            data_size = 5000;
        }
        else
        {
            data_size = enc_length - total_received;
        }
        received = recv(socket_fd, enc_buffer + index, data_size, 0);
        if (received < 0)
        {
            fprintf(stderr, "Client: Error reading from server\n");
            exit(2);
        }
        total_received += received;
        if (total_received >= enc_length)
        {
            is_done = 1;
        }
        index += received;
    }

    // Print encrypted message
    printf("%s\n", enc_buffer);

    // Clean up
    close(socket_fd);
    free(text_buffer);
    free(key_buffer);
    free(enc_buffer);

    return 0;
}

