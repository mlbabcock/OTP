#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>


char key_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ ";
char *text_buffer, *key_buffer, *dec_buffer;
int text_size, key_size;


//Function declarations
void is_valid_file(const char *text, const char *key);
void address_struct(struct sockaddr_in *address, int port_num);
int initial_contact(int socket_fd);
void send_text_msg(int socket_fd);
void send_key_msg(int socket_fd);
void receive_msg(int socket_fd);

int main(int argc, char *argv[])
{
    int socket_fd, port_num, chars_written, chars_read;
    struct sockaddr_in server_address;

    // Check if the correct number of command-line arguments is provided
    if (argc < 4)
    {
        fprintf(stderr, "USAGE: %s plaintext keytext port\n", argv[0]);
        exit(1);
    }
    is_valid_file(argv[1], argv[2]);                                                            // Validate the text and key files
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);                                                // Create a socket
    if (socket_fd < 0)
    {
        fprintf(stderr, "CLIENT: Error opening socket\n");
        exit(2);
    }

    address_struct(&server_address, atoi(argv[3]));                                             // Set up the server address structure

    // Connect to the server
    if (connect(socket_fd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
    {
        fprintf(stderr, "CLIENT: Error connecting\n");
        exit(2);
    }

    // Perform initial contact with the server
    if (initial_contact(socket_fd) == 0)
    {
        fprintf(stderr, "CLIENT: Error Connection is rejected by server.\n");
        exit(2);
    }
    send_text_msg(socket_fd);                                                               // Send the text message to the server
    send_key_msg(socket_fd);                                                                // Send the key message to the server
    receive_msg(socket_fd);                                                                 // Receive and store the decrypted message from the server
    printf("%s\n", dec_buffer);                                                             // Print the decrypted message
    close(socket_fd);                                                                       // Close the socket
    return 0;
}

// Function to validate text and key files
void is_valid_file(const char *text, const char *key)
{
    // Structures to store file information
    struct stat text_Stat;
    struct stat key_Stat;

    // Get file information
    stat(text, &text_Stat);
    stat(key, &key_Stat);
    text_size = text_Stat.st_size;
    key_size = key_Stat.st_size;

    // Open text and key files
    FILE *text_file = fopen(text, "r");
    FILE *key_file = fopen(key, "r");

    // Check if files are successfully opened
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

    // Allocate memory for text and key buffers
    text_buffer = (char *)calloc(text_size + 1, sizeof(char));
    key_buffer = (char *)calloc(key_size + 1, sizeof(char));

    // Initialize buffers and read data from files
    memset(text_buffer, '\0', text_size);
    memset(key_buffer, '\0', key_size);
    fgets(text_buffer, text_size, text_file);
    fgets(key_buffer, key_size, key_file);
    fclose(text_file);
    fclose(key_file);

    // Update text and key sizes
    text_size = strlen(text_buffer);
    key_size = strlen(key_buffer);

    // Check if key is shorter than the text
    if (text_size > key_size)
    {
        fprintf(stderr, "Error: key '%s' is too short\n", key);
        exit(1);
    }

    // Null-terminate the buffers
    text_buffer[text_size] = '\0';
    key_buffer[key_size] = '\0';

    // Check for bad characters in text and key
    for (int i = 0; i < text_size; ++i)
    {
        if (strchr(key_chars, text_buffer[i]) == NULL)
        {
            fprintf(stderr, "Error: '%s' has a bad character\n", text);
            exit(1);
        }
    }
    for (int i = 0; i < key_size; ++i)
    {
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
    memset((char *)address, '\0', sizeof(*address));

    address->sin_family = AF_INET;
    address->sin_port = htons(port_num);

    struct hostent *hostInfo = gethostbyname(ip_address);                               // Get host information
    if (hostInfo == NULL)
    {
        fprintf(stderr, "CLIENT: Error, no such host\n");
        exit(2);
    }
    memcpy((char *)&address->sin_addr.s_addr, hostInfo->h_addr_list[0], hostInfo->h_length);
}

// Function for initial contact with the server
int initial_contact(int socket_fd)
{
    int response;
    char name[] = "dec_client\0";

    // Send an initial message to the server
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

// Function to send the text message to the server
void send_text_msg(int socket_fd)
{
    // Send the size of the text
    if (send(socket_fd, &text_size, sizeof(text_size), 0) < 0)
    {
        fprintf(stderr, "Client: Error failed to send an initial message to server\n");
        exit(2);
    }

    // Send the text content
    if (send(socket_fd, text_buffer, text_size, 0) < 0)
    {
        fprintf(stderr, "Client: Error failed to send an initial message to server\n");
        exit(2);
    }
}

// Function to send the key message to the server
void send_key_msg(int socket_fd)
{
    // Send the size of the key
    if (send(socket_fd, &key_size, sizeof(key_size), 0) < 0)
    {
        fprintf(stderr, "Client: Error failed to send an initial message to server\n");
        exit(2);
    }

    // Send the key content
    if (send(socket_fd, key_buffer, key_size, 0) < 0)
    {
        fprintf(stderr, "Client: Error failed to send an initial message to server\n");
        exit(2);
    }
}

// Function to receive decrypted message from the server
void receive_msg(int socket_fd)
{
    int dec_length, total_received = 0, received, index = 0;

    // Receive the length of the decrypted message
    if (recv(socket_fd, &dec_length, sizeof(dec_length), 0) < 0)
    {
        fprintf(stderr, "Client: Error reading from server\n");
        exit(2);
    }

    // Allocate memory for the decrypted message buffer
    dec_buffer = (char *)calloc(dec_length + 1, sizeof(char));
    memset(dec_buffer, '\0', dec_length + 1);

    int is_done = 0;
    int data_size;

    // Receive the decrypted message in chunks
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
}
