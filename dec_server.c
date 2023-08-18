#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

// The characters used for encryption/decryption
char key_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ ";

// Buffers for holding text, key, and decrypted messages
char *text_buffer, *key_buffer, *dec_buffer;

int main(int argc, char *argv[])
{
    // Socket variables
    int connect_socket, chars_read;
    struct sockaddr_in server_address, client_address;
    socklen_t sizeOfClientInfo = sizeof(client_address);

    // Ignore child process signals to prevent zombies
    struct sigaction action;
    memset(&action, 0, sizeof(action));
    action.sa_handler = SIG_IGN;
    sigaction(SIGCHLD, &action, NULL);

    // Check command line arguments
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

    // Set up server address
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(atoi(argv[1]));
    server_address.sin_addr.s_addr = INADDR_ANY;

    // Bind the socket to the server address
    if (bind(listen_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
    {
        fprintf(stderr, "Server: Error on binding\n");
        exit(2);
    }

    // Start listening for incoming connections
    listen(listen_socket, 5);

    // Main server loop
    while (1)
    {
        // Accept a new connection
        connect_socket = accept(listen_socket, (struct sockaddr *)&client_address, &sizeOfClientInfo);
        if (connect_socket < 0)
        {
            fprintf(stderr, "Server: ERROR on accept\n");
            exit(2);
        }

        // Fork a child process to handle the client
        pid_t spawnPid;
        spawnPid = fork();

        switch (spawnPid)
        {
        case -1:
            fprintf(stderr, "Server: Error fork() failed\n");
            exit(1);
            break;
        case 0:
            close(listen_socket);

            // ... (code for handling client communication and decryption)

            // Clean up and exit the child process
            close(connect_socket);
            exit(0);
            break;
        default:
            break;
        }
        close(connect_socket);
    }
    close(listen_socket);
    return 0;
}
