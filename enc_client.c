#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

char key_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ ";
char *text_buffer, *key_buffer, *enc_buffer;
int text_size, key_size;

// Function Declarations 
void is_valid_file(const char *text, const char *key);
void address_struct(struct sockaddr_in *address, int port_num);
int initial_contact(int socket_fd);
void send_text_msg(int socket_fd);
void send_key_msg(int socket_fd);
void receive_msg(int socket_fd);

int main(int argc, char *argv[])
{
    // Variable declarations
    int socket_fd, port_num, chars_written, chars_read;
    struct sockaddr_in server_address;

    // Check command-line arguments
    if (argc < 4)
    {
        fprintf(stderr, "USAGE: %s plaintext keytext port\n", argv[0]);
        exit(1);
    }
    is_valid_file(argv[1], argv[2]);                                                // Validate input files
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);                                    // Create a socket
    if (socket_fd < 0)
    {
        fprintf(stderr, "CLIENT: Error opening socket\n");
        exit(2);
    }

    address_struct(&server_address, atoi(argv[3]));                                 // Set up server address structure

    // Connect to the server
    if (connect(socket_fd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
    {
        fprintf(stderr, "CLIENT: Error connecting\n");
        exit(2);
    }

    // Establish initial contact with the server
    if (initial_contact(socket_fd) == 0)
    {
        fprintf(stderr, "CLIENT: Error Connection is rejected by server.\n");
        exit(2);
    }

    send_text_msg(socket_fd);                                                   // Send text message to the server
    send_key_msg(socket_fd);                                                    // Send key message to the server
    receive_msg(socket_fd);                                                     // Receive encrypted message from the server
    printf("%s\n", enc_buffer);                                                 // Print the decrypted message
    close(socket_fd);                                                           // Close the socket and exit
    return 0;
}

// Function to validate input files
void is_valid_file(const char *text, const char *key)
{
    // File size structures
    struct stat text_Stat;
    struct stat key_Stat;

    // Get file stats
    stat(text, &text_Stat);
    stat(key, &key_Stat);
    text_size = text_Stat.st_size;
    key_size = key_Stat.st_size;

    // Open text and key files
    FILE *text_file = fopen(text, "r");
    FILE *key_file = fopen(key, "r");

    // Handle file open errors
    if (text_file == NULL)
    {
        fprintf(stderr, "Error: Cannot open %s\n", text);
        exit(1);
    }
    else if (key_file == NULL)
    {
        fprintf(stderr, "Error: Cannot open %s\n", key);
        exit(1);
    }

    // Allocate memory for buffers and read file contents
    text_buffer = (char *)calloc(text_size + 1, sizeof(char));
    key_buffer = (char *)calloc(key_size + 1, sizeof(char));
    memset(text_buffer, '\0', text_size);
    memset(key_buffer, '\0', key_size);

    fgets(text_buffer, text_size, text_file);                                               // Read text from file into buffer
    fgets(key_buffer, key_size, key_file);                                                  // Read key from file into buffer
    fclose(text_file);                                                                      // Close text and key files
    fclose(key_file);

    // Update sizes and validate text and key characters
    text_size = strlen(text_buffer);
    key_size = strlen(key_buffer);

     // Check characters and sizes
    if (text_size > key_size)
    {
        fprintf(stderr, "Error: key '%s' is too short\n", key);
        exit(1);
    }

    // Null-terminate text and key buffers
    text_buffer[text_size] = '\0';
    key_buffer[key_size] = '\0';

    for (int i = 0; i < text_size; ++i)
    {
        // Check if text contains valid characters
        if (strchr(key_chars, text_buffer[i]) == NULL)
        {
            fprintf(stderr, "Error: '%s' has a bad character\n", text);
            exit(1);
        }
    }
    for (int i = 0; i < key_size; ++i)
    {
        // Check if key contains valid characters
        if (strchr(key_chars, key_buffer[i]) == NULL)
        {
            fprintf(stderr, "Error: '%s' has a bad character\n", key);
            exit(1);
        }
    }
}

// Function to set up server address structure
void address_struct(struct sockaddr_in *address, int port_num)
{
    char ip_address[] = "127.0.0.1";
    memset((char *)address, '\0', sizeof(*address));                               // Clear address structure

    // Set address family and port
    address->sin_family = AF_INET;
    address->sin_port = htons(port_num);

    struct hostent *host_info = gethostbyname(ip_address);                        // Get host information
    if (host_info == NULL)
    {
        fprintf(stderr, "Client: Error, no such host\n");
        exit(2);
    }
    // Copy IP address to address structure
    memcpy((char *)&address->sin_addr.s_addr, host_info->h_addr_list[0], host_info->h_length);
}

// Function for initial contact with the server
int initial_contact(int socket_fd)
{
    int response;
    char name[] = "enc_client\0";
    // Send client name to the server
    if (send(socket_fd, name, strlen(name), 0) < 0)
    {
        fprintf(stderr, "Client: Error failed to send an initial message to server\n");
        exit(2);
    }

    // Receive response from the server
    if (recv(socket_fd, &response, sizeof(response), 0) < 0)
    {
        fprintf(stderr, "Client: Error failed to receive an initial message from server\n");
        exit(2);
    }
    return response;
}

// Function to send text message to the server
void send_text_msg(int socket_fd)
{
    // Send text size to the server
    if (send(socket_fd, &text_size, sizeof(text_size), 0) < 0)
    {
        fprintf(stderr, "Client: Error failed to send an initial message to server\n");
        exit(2);
    }

    // Send text content to the server
    if (send(socket_fd, text_buffer, text_size, 0) < 0)
    {
        fprintf(stderr, "Client: Error failed to send an initial message to server\n");
        exit(2);
    }
}

// Function to send key message to the server
void send_key_msg(int socket_fd)
{
    // Send key size to the server
    if (send(socket_fd, &key_size, sizeof(key_size), 0) < 0)
    {
        fprintf(stderr, "Client: Error failed to send an initial message to server\n");
        exit(2);
    }

    // Send key content to the server
    if (send(socket_fd, key_buffer, key_size, 0) < 0)
    {
        fprintf(stderr, "Client: Error failed to send an initial message to server\n");
        exit(2);
    }
}

// Function to receive encrypted message from the server
void receive_msg(int socket_fd)
{
    int enc_length, total_received = 0, received, index = 0;

    // Receive encrypted message length from the server
    if (recv(socket_fd, &enc_length, sizeof(enc_length), 0) < 0)
    {
        fprintf(stderr, "Client: Error reading from server\n");
        exit(2);
    }
    enc_buffer = (char *)calloc(enc_length + 1, sizeof(char));                          // Allocate memory for encrypted buffer
    memset(enc_buffer, '\0', enc_length + 1);

    int is_done = 0;
    int data_size;

    // Receive encrypted message data in chunks
    while (!is_done)
    {
        // Determine the amount of data to receive in this iteration
        if (enc_length - total_received >= 5000)
        {
            // If remaining data is larger than or equal to 5000 bytes, receive 5000 bytes
            data_size = 5000;
        }
        else
        {
            // If remaining data is less than 5000 bytes, receive the remaining amount
            data_size = enc_length - total_received;
        }

        // Receive data from the server into the encryption buffer starting at 'index'
        received = recv(socket_fd, enc_buffer + index, data_size, 0);

        if (received < 0)                                                           // Check for errors while receiving data
        {
            fprintf(stderr, "Client: Error reading from server\n");
            exit(2);
        }
        total_received += received;                                                 // Update the total amount of received data
        if (total_received >= enc_length)                                           // Check if all expected data has been received
        {
            is_done = 1;                                                            // Set the flag to indicate that data reception is complete
        }
        index += received;                                                          // Update the index to the position after the received data
    }
