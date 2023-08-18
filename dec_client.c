#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

// The characters used for encryption/decryption
char key_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ ";

// Buffers for holding text, key, and decrypted messages
char *text_buffer, *key_buffer, *dec_buffer;
int text_size, key_size;

int main(int argc, char *argv[])
{
    int socket_fd, port_number, chars_written, chars_read;
    struct sockaddr_in server_address;

    // Check command line arguments
    if (argc < 4)
    {
        fprintf(stderr, "USAGE: %s plaintext keytext port\n", argv[0]);
        exit(1);
    }

    struct stat text_Stat;
    struct stat key_Stat;

    // Get file sizes for text and key files
    stat(argv[1], &text_Stat);
    stat(argv[2], &key_Stat);
    text_size = text_Stat.st_size;
    key_size = key_Stat.st_size;

    // Open text and key files
    FILE *text_file = fopen(argv[1], "r");
    FILE *key_file = fopen(argv[2], "r");

    // Error handling for file open
    if (text_file == NULL)
    {
        fprintf(stderr, "Error: Cannot open %s\n", argv[1]);
        exit(1);
    }
    else if (key_file == NULL)
    {
        fprintf(stderr, "Error: Cannot open %s\n", argv[2]);
        exit(1);
    }

    // Read text and key data into buffers
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

    // Check key size
    if (text_size > key_size)
    {
        fprintf(stderr, "Error: key '%s' is too short\n", argv[2]);
        exit(1);
    }

    text_buffer[text_size] = '\0';
    key_buffer[key_size] = '\0';

    // Check for invalid characters in text and key
    for (int i = 0; i < text_size; ++i)
    {
        if (strchr(key_chars, text_buffer[i]) == NULL)
        {
            fprintf(stderr, "Error: '%s' has an invalid character\n", argv[1]);
            exit(1);
        }
    }
    for (int i = 0; i < key_size; ++i)
    {
        if (strchr(key_chars, key_buffer[i]) == NULL)
        {
            fprintf(stderr, "Error: '%s' has an invalid character\n", argv[2]);
            exit(1);
        }
    }

    // Create a socket for communication
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

    // Get host information
    struct hostent *hostInfo = gethostbyname("127.0.0.1");
    if (hostInfo == NULL)
    {
        fprintf(stderr, "CLIENT: Error, no such host\n");
        exit(2);
    }
    memcpy((char *)&server_address.sin_addr.s_addr, hostInfo->h_addr_list[0], hostInfo->h_length);

    // Connect to the server
    if (connect(socket_fd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
    {
        fprintf(stderr, "CLIENT: Error connecting\n");
        exit(2);
    }

    // Initial contact with the server
    int response;
    char name[] = "dec_client\0";
    if (send(socket_fd, name, strlen(name), 0) < 0)
    {
        fprintf(stderr, "Client: Error Failed to send an initial message to server\n");
        exit(2);
    }

    // Receive response from the server
    if (recv(socket_fd, &response, sizeof(response), 0) < 0)
    {
        fprintf(stderr, "Client: Error Failed to receive an initial message from server\n");
        exit(2);
    }

    // Check if connection is rejected
    if (response == 0)
    {
        fprintf(stderr, "CLIENT: Error Connection rejected by server.\n");
        exit(2);
    }

    // Send text size and text data to the server
    if (send(socket_fd, &text_size, sizeof(text_size), 0) < 0)
    {
        fprintf(stderr, "Client: Error Failed to send text size to server\n");
        exit(2);
    }
    if (send(socket_fd, text_buffer, text_size, 0) < 0)
    {
        fprintf(stderr, "Client: Error Failed to send text data to server\n");
        exit(2);
    }

    // Send key size and key data to the server
    if (send(socket_fd, &key_size, sizeof(key_size), 0) < 0)
    {
        fprintf(stderr, "Client: Error Failed to send key size to server\n");
        exit(2);
    }
    if (send(socket_fd, key_buffer, key_size, 0) < 0)
    {
        fprintf(stderr, "Client: Error Failed to send key data to server\n");
        exit(2);
    }

    // Receive decrypted message from the server
    int dec_length, total_received = 0, received, index = 0;
    if (recv(socket_fd, &dec_length, sizeof(dec_length), 0) < 0)
    {
        fprintf(stderr, "Client: Error reading from server\n");
        exit(2);
    }
    dec_buffer = (char *)calloc(dec_length + 1, sizeof(char));
    memset(dec_buffer, '\0', dec_length + 1);

    int is_done = 0;
    int data_size;

    // Receive the decrypted message data in chunks
    while (!is_done)
    {
        if (dec_length - total_received >= 5000)
        {
            data_size = 5000;
        }
        else
        {
            data_size = dec_length - total_received;
        }
        received = recv(socket_fd, dec_buffer + index, data_size, 0);
        if (received < 0)
        {
            fprintf(stderr, "Client: Error reading from server\n");
            exit(2);
        }
        total_received += received;
        if (total_received >= dec_length)
        {
            is_done = 1;
        }
        index += received;
    }

    // Print the decrypted message
    printf("%s\n", dec_buffer);

    // Clean up and close the socket
    close(socket_fd);
    return 0;
}
