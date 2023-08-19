/*
    Sources Used: 
    https://research.nii.ac.jp/~ichiro/syspro98 
    Explorations, Provided Code 
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

char key_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ ";
char *text_buffer, *key_buffer, *dec_buffer;                        // Declare buffers for storing text, key, and decrypted data

// Signal handler to catch child process termination
void catch_child_signal(int signo)
{
    pid_t child_pid = 0;
    do
    {
        int child_ret;
        child_pid = waitpid(-1, &child_ret, WNOHANG);                                   // Wait for child processes to terminate
    } while (child_pid > 0);
}

// Function Declarations 
void address_struct(struct sockaddr_in *address, int port_num);
void child_process(int connect_socket);
void to_decrypt();
int initial_contact(int connect_socket);
void send_result(int connect_socket);
void receive_input_msg(int connect_socket);
void receive_key_msg(int connect_socket);

int main(int argc, char *argv[])
{
    int connect_socket, chars_read;
    struct sockaddr_in server_address, client_address;
    socklen_t sizeOfClientInfo = sizeof(client_address);

    struct sigaction action;
    memset(&action, 0, sizeof(action));
    action.sa_handler = catch_child_signal;
    sigemptyset(&action.sa_mask);
    action.sa_flags = SA_NOCLDSTOP | SA_RESTART;
    sigaction(SIGCHLD, &action, NULL);                                          // Set up signal handler for child process termination

    if (argc < 2)
    {
        fprintf(stderr, "USAGE: %s port\n", argv[0]);
        exit(1);
    }

    // Create a socket for listening
    int listen_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_socket < 0)
    {
        fprintf(stderr, "Server: Error opening socket");
        exit(2);
    }

    address_struct(&server_address, atoi(argv[1]));                                  // Set up server address structure

    // Bind the socket to the server address
    if (bind(listen_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
    {
        fprintf(stderr, "Server: Error on binding\n");
        exit(2);
    }

    listen(listen_socket, 5);                                                      // Start listening for incoming connections

    while (1)
    {
        // Accept incoming connections
        connect_socket = accept(listen_socket, (struct sockaddr *)&client_address, &sizeOfClientInfo);
        if (connect_socket < 0)
        {
            fprintf(stderr, "Server: Error on accept\n");
            exit(2);
        }

        pid_t spawnPid;
        spawnPid = fork();                                                  // Fork a child process to handle the connection

        switch (spawnPid)
        {
        case -1:
            fprintf(stderr, "Server: Error fork() failed\n");
            exit(1);
            break;
        case 0:
            close(listen_socket);                                           // Close listening socket in child process
            child_process(connect_socket);                                  // Handle the client connection
            close(connect_socket);                                          // Close the connection socket in child process
            exit(0);
            break;
        default:
            break;
        }
        close(connect_socket);                                              // Close connection socket in parent process
    }
    close(listen_socket);                                                   // Close listening socket in parent process
    return 0;
}

// Set up the address structure for the server
void address_struct(struct sockaddr_in *address, int port_num)
{
    memset((char *)address, '\0', sizeof(*address));
    address->sin_family = AF_INET;
    address->sin_port = htons(port_num);
    address->sin_addr.s_addr = INADDR_ANY;
}

// Child process function to handle communication with client
void child_process(int connect_socket)
{
    if (initial_contact(connect_socket) == 0)                       // Perform initial contact with the client
    {
        close(connect_socket);
        exit(2);
    }
    receive_input_msg(connect_socket);                              // Receive input text and key from the client
    receive_key_msg(connect_socket);

    to_decrypt();                                                   // Decrypt the input text using the key
    send_result(connect_socket);                                    // Send the decrypted result back to the client
}

// Perform initial contact with the client
int initial_contact(int connect_socket)
{
    char recv_name[256];
    int reply;
    memset(recv_name, '\0', 256);

    if (recv(connect_socket, recv_name, 255, 0) < 0)                            // Receive client's initial message
    {
        fprintf(stderr, "Server: Error Failed to receive an initial message from client\n");
        exit(2);
    }

    // Check if the received message is the expected client identifier
    if (strcmp(recv_name, "dec_client\0") == 0)
    {
        reply = 1;
    }
    else
    {
        reply = 0;
    }
    if (send(connect_socket, &reply, sizeof(reply), 0) < 0)                                 // Send reply to the client
    {
        fprintf(stderr, "Server: Error Failed to send an initial message to client\n");
        exit(2);
    }
    return reply;
}

// Decrypt the input text using the provided key
void to_decrypt()
{
    // Determine the index of characters in the key and text
    int text_size = strlen(text_buffer);
    dec_buffer = (char *)calloc(text_size + 1, sizeof(char));                       // Allocate memory for the decrypted buffer
    memset(dec_buffer, '\0', text_size + 1);                                        // Initialize the decrypted buffer with null characters
    for (int i = 0; i < text_size; ++i)                                             // Determine the index of characters in the key and text
    {
        int text_index = strchr(key_chars, text_buffer[i]) - key_chars, key_index = strchr(key_chars, key_buffer[i]) - key_chars;

        // Decrypt the character and store in the result buffer
        dec_buffer[i] = key_chars[(strlen(key_chars) + text_index - key_index) % strlen(key_chars)];
    }
}

// Send the decrypted result back to the client
void send_result(int connect_socket)
{
    // Send the size of the decrypted message to the client
    int dec_size = strlen(dec_buffer);
    if (send(connect_socket, &dec_size, sizeof(dec_size), 0) < 0)
    {
        fprintf(stderr, "Server: Error Failed to send an initial message to Client\n");
        exit(2);
    }

    // Send the decrypted message itself to the client
    if (send(connect_socket, dec_buffer, dec_size, 0) < 0)
    {
        fprintf(stderr, "Server: Error Failed to send an initial message to Client\n");
        exit(2);
    }
}

// Receive the input text from the client
void receive_input_msg(int connect_socket)
{
    int text_length, total_received = 0, received, index = 0;

    // Receive the length of the input text from the client
    if (recv(connect_socket, &text_length, sizeof(text_length), 0) < 0)
    {
        fprintf(stderr, "Server: Error reading from client\n");
        exit(2);
    }
    text_buffer = (char *)calloc(text_length + 1, sizeof(char));                            // Allocate memory to store the input text
    memset(text_buffer, '\0', text_length + 1);                                             // Initialize the text buffer with null characters

    int is_done = 0;
    int data_size;
    while (!is_done)
    {
        // Determine the amount of data to receive in this iteration
        if (text_length - total_received >= 5000)
        {
            data_size = 5000;
        }
        else
        {
            data_size = text_length - total_received;
        }
        received = recv(connect_socket, text_buffer + index, data_size, 0);                         // Receive data from the client
        if (received < 0)
        {
            fprintf(stderr, "Server: Error reading from client\n");
            exit(2);
        }
        total_received += received;                                                         // Update counters and check if all data has been received
        if (total_received >= text_length)
        {
            is_done = 1;
        }
        index += received;
    }
}

// Receive the encryption key from the client
void receive_key_msg(int connect_socket)
{
    // Receive the length of the encryption key from the client
    int key_length, total_received = 0, received, index = 0;
    if (recv(connect_socket, &key_length, sizeof(key_length), 0) < 0)
    {
        fprintf(stderr, "Error reading from socket\n");
        exit(2);
    }

    key_buffer = (char *)calloc(key_length + 1, sizeof(char));                          // Allocate memory to store the encryption key
    memset(key_buffer, '\0', key_length + 1);                                           // Initialize the key buffer with null characters

    int is_done = 0;
    int data_size;

    while (!is_done)
    {
        // Determine the amount of data to receive in this iteration
        if (key_length - total_received >= 5000)
        {
            data_size = 5000;
        }
        else
        {
            data_size = key_length - total_received;
        }
        received = recv(connect_socket, key_buffer + index, data_size, 0);                      // Receive data from the client
        if (received < 0)
        {
            fprintf(stderr, "Error reading from socket\n");
            exit(2);
        }
        total_received += received;                                                         // Update counters and check if all data has been received
        if (total_received >= key_length)
        {
            is_done = 1;
        }
        index += received;
    }
}
