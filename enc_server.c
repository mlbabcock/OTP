/*
    Sources Used:
    https://research.nii.ac.jp/~ichiro/syspro98
    Explorations, Code Provided 
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

// Declare global buffers for input, key, and encrypted output
char *input_buffer, *key_buffer, *enc_output_buffer;

// Function to handle child termination signals
void catch_child_signal(int signo)
{
    pid_t child_pid = 0;
    do
    {
        int child_ret;
        child_pid = waitpid(-1, &child_ret, WNOHANG);                       // Reap all child processes that have terminated
    } while (child_pid > 0);
}

// Function Declarations 
void address_struct(struct sockaddr_in *address, int port_num);
void child_process(int connect_socket);
void to_encrypt();
int initial_contact(int connect_socket);
void send_result(int connect_socket);
void receive_input_msg(int connect_socket);
void receive_key_msg(int connect_socket);

int main(int argc, char *argv[])
{
    int connect_socket, chars_read;
    struct sockaddr_in server_address, client_address;
    socklen_t sizeOfClientInfo = sizeof(client_address);

    // Configure signal handling for child termination
    struct sigaction action;
    memset(&action, 0, sizeof(action));
    action.sa_handler = catch_child_signal;
    sigemptyset(&action.sa_mask);
    action.sa_flags = SA_NOCLDSTOP | SA_RESTART;
    sigaction(SIGCHLD, &action, NULL);                              // Attach the signal handler to SIGCHLD (child termination) signal

    // Check command-line arguments
    if (argc < 2)
    {
        fprintf(stderr, "USAGE: %s port\n", argv[0]);
        exit(1);
    }

    // Create a listening socket
    int listen_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_socket < 0)
    {
        fprintf(stderr, "Server: Error opening socket");
        exit(2);
    }

    // Configure server address and bind the socket
    address_struct(&server_address, atoi(argv[1]));

    if (bind(listen_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
    {
        fprintf(stderr, "Server: Error on binding\n");
        exit(2);
    }

    // Start listening for incoming connections
    listen(listen_socket, 5);

    // Accept and handle incoming connections
    while (1)
    {
        // Accept a connection request and get a new socket for communication
        connect_socket = accept(listen_socket, (struct sockaddr *)&client_address, &sizeOfClientInfo);
        if (connect_socket < 0)
        {
            fprintf(stderr, "Server: Error on accept\n");
            exit(2);
        }

        pid_t spawnPid;
        spawnPid = fork();                                          // Create a child process to handle the client's request

        switch (spawnPid)
        {
        case -1:
            fprintf(stderr, "Server: Error fork() failed\n");
            exit(1);
            break;
        case 0:                                                             // Child process
            close(listen_socket);                                           // Close the listening socket in the child process
            child_process(connect_socket);
            close(connect_socket);
            exit(0);
            break;
        default:
            // Parent process
            break;
        }
        close(connect_socket);                                              // Close the socket in the parent process
    }
    close(listen_socket);                                                   // Close the listening socket when the server terminates
    return 0;
}

// Configure address structure
void address_struct(struct sockaddr_in *address, int port_num)
{
    memset((char *)address, '\0', sizeof(*address));
    address->sin_family = AF_INET;
    address->sin_port = htons(port_num);
    address->sin_addr.s_addr = INADDR_ANY;
}

// Child process function
void child_process(int connect_socket)
{
    // Perform initial contact with the client
    if (initial_contact(connect_socket) == 0)
    {
        close(connect_socket);
        exit(2);
    }
    // Receive input and key messages from client
    receive_input_msg(connect_socket);
    receive_key_msg(connect_socket);
    to_encrypt();                                           // Encrypt the input using the key
    send_result(connect_socket);                            // Send the encrypted result back to the client
}

// Initial contact function
int initial_contact(int connect_socket)
{
    char recv_name[256];
    int reply;
    memset(recv_name, '\0', 256);

    // Receive client's initial message
    if (recv(connect_socket, recv_name, 255, 0) < 0)
    {
        fprintf(stderr, "Server: Error failed to receive an initial message from client\n");
        exit(2);
    }

    // Check if the received message matches the expected client name
    if (strcmp(recv_name, "enc_client\0") == 0)
    {
        reply = 1;
    }
    else
    {
        reply = 0;
    }

    // Send reply to the client
    if (send(connect_socket, &reply, sizeof(reply), 0) < 0)
    {
        fprintf(stderr, "Server: Error failed to send an initial message to client\n");
        exit(2);
    }
    return reply;
}

// Function to perform encryption
void to_encrypt()
{
    int text_size = strlen(input_buffer);
    enc_output_buffer = (char *)calloc(text_size + 1, sizeof(char));
    memset(enc_output_buffer, '\0', text_size + 1);

    // Perform encryption on each character of the input using the key
    for (int i = 0; i < text_size; ++i)
    {
        int text_index = strchr(key_chars, input_buffer[i]) - key_chars, key_index = strchr(key_chars, key_buffer[i]) - key_chars;
        enc_output_buffer[i] = key_chars[(text_index + key_index) % strlen(key_chars)];
    }
}

// Function to send encrypted result to the client
void send_result(int connect_socket)
{
    int enc_size = strlen(enc_output_buffer);
    if (send(connect_socket, &enc_size, sizeof(enc_size), 0) < 0)
    {
        fprintf(stderr, "Server: Error failed to send an initial message to Client\n");
        exit(2);
    }

    if (send(connect_socket, enc_output_buffer, enc_size, 0) < 0)
    {
        fprintf(stderr, "Server: Error failed to send an initial message to Client\n");
        exit(2);
    }
}

// Function to receive input message from the client
void receive_input_msg(int connect_socket)
{
    int text_length, total_received = 0, received, index = 0;
    if (recv(connect_socket, &text_length, sizeof(text_length), 0) < 0)
    {
        fprintf(stderr, "Server: Error reading from client\n");
        exit(2);
    }

    // Allocate memory to store the input message
    input_buffer = (char *)calloc(text_length + 1, sizeof(char));
    memset(input_buffer, '\0', text_length + 1);

    int is_done = 0;
    int data_size;

    // Receive input data in chunks until all data is received
    while (!is_done)
    {
        // Determine the size of data to receive in this iteration
        if (text_length - total_received >= 5000)
        {
            data_size = 5000;
        }
        else
        {
            data_size = text_length - total_received;
        }

        // Receive data from the client and store it in the input buffer
        received = recv(connect_socket, input_buffer + index, data_size, 0);
        if (received < 0)
        {
            fprintf(stderr, "Server: Error reading from client\n");
            exit(2);
        }
        total_received += received;
        if (total_received >= text_length)
        {
            is_done = 1;
        }
        index += received;
    }
}

// Function to receive key message from the client
void receive_key_msg(int connect_socket)
{

    int key_length, total_received = 0, received, index = 0;
    if (recv(connect_socket, &key_length, sizeof(key_length), 0) < 0)
    {
        fprintf(stderr, "Error reading from socket\n");
        exit(2);
    }

    // Allocate memory to store the key message
    key_buffer = (char *)calloc(key_length + 1, sizeof(char));
    memset(key_buffer, '\0', key_length + 1);

    int is_done = 0;
    int data_size;

    // Receive key data in chunks until all data is received
    while (!is_done)
    {
        // Determine the size of data to receive in this iteration
        if (key_length - total_received >= 5000)
        {
            data_size = 5000;
        }
        else
        {
            data_size = key_length - total_received;
        }

        // Receive data from the client and store it in the key buffer
        received = recv(connect_socket, key_buffer + index, data_size, 0);
        if (received < 0)
        {
            fprintf(stderr, "Error reading from socket\n");
            exit(2);
        }

        total_received += received;
        if (total_received >= key_length)
        {
            is_done = 1;
        }
        index += received;
    }
}
