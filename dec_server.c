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
char *text_buffer, *key_buffer, *dec_buffer;

int main(int argc, char *argv[])
{
    int connect_socket, chars_read;
    struct sockaddr_in server_address, client_address;
    socklen_t sizeOfClientInfo = sizeof(client_address);

    struct sigaction action;
    memset(&action, 0, sizeof(action));
    action.sa_handler = SIG_IGN; 
    sigaction(SIGCHLD, &action, NULL);

    if (argc < 2)
    {
        fprintf(stderr, "USAGE: %s port\n", argv[0]);
        exit(1);
    }

    int listen_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_socket < 0)
    {
        fprintf(stderr, "Server: Error opening socket");
        exit(2);
    }

    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(atoi(argv[1]));
    server_address.sin_addr.s_addr = INADDR_ANY;

    if (bind(listen_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
    {
        fprintf(stderr, "Server: Error on binding\n");
        exit(2);
    }

    listen(listen_socket, 5);

    while (1)
    {
        connect_socket = accept(listen_socket, (struct sockaddr *)&client_address, &sizeOfClientInfo);
        if (connect_socket < 0)
        {
            fprintf(stderr, "Server: ERROR on accept\n");
            exit(2);
        }

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

            // Initial contact
            char recv_name[256];
            int reply;
            memset(recv_name, '\0', 256);

            if (recv(connect_socket, recv_name, 255, 0) < 0)
            {
                fprintf(stderr, "Server: Error Failed to receive an initial message from client\n");
                exit(2);
            }

            if (strcmp(recv_name, "dec_client\0") == 0)
            {
                reply = 1;
            }
            else
            {
                reply = 0;
            }
            if (send(connect_socket, &reply, sizeof(reply), 0) < 0)
            {
                fprintf(stderr, "Server: Error Failed to send an initial message to client\n");
                exit(2);
            }

            // Receive input message
            int text_length, total_received = 0, received, index = 0;
            if (recv(connect_socket, &text_length, sizeof(text_length), 0) < 0)
            {
                fprintf(stderr, "Server: Error reading from client\n");
                exit(2);
            }
            text_buffer = (char *)calloc(text_length + 1, sizeof(char));
            memset(text_buffer, '\0', text_length + 1);

            int is_done = 0;
            int data_size;
            while (!is_done)
            {
                if (text_length - total_received >= 5000)
                {
                    data_size = 5000;
                }
                else
                {
                    data_size = text_length - total_received;
                }
                received = recv(connect_socket, text_buffer + index, data_size, 0);
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

            // Receive key message
            int key_length, total_received_key = 0, received_key, index_key = 0;
            if (recv(connect_socket, &key_length, sizeof(key_length), 0) < 0)
            {
                fprintf(stderr, "Error reading from socket\n");
                exit(2);
            }

            key_buffer = (char *)calloc(key_length + 1, sizeof(char));
            memset(key_buffer, '\0', key_length + 1);

            is_done = 0;

            while (!is_done)
            {
                if (key_length - total_received_key >= 5000)
                {
                    data_size = 5000;
                }
                else
                {
                    data_size = key_length - total_received_key;
                }
                received_key = recv(connect_socket, key_buffer + index_key, data_size, 0);
                if (received_key < 0)
                {
                    fprintf(stderr, "Error reading from socket\n");
                    exit(2);
                }
                total_received_key += received_key;
                if (total_received_key >= key_length)
                {
                    is_done = 1;
                }
                index_key += received_key;
            }

            int text_size = strlen(text_buffer);
            dec_buffer = (char *)calloc(text_size + 1, sizeof(char));
            memset(dec_buffer, '\0', text_size + 1);
            for (int i = 0; i < text_size; ++i)
            {
                int text_index = strchr(key_chars, text_buffer[i]) - key_chars, key_index = strchr(key_chars, key_buffer[i]) - key_chars;
                dec_buffer[i] = key_chars[(strlen(key_chars) + text_index - key_index) % strlen(key_chars)];
            }

            // Send decrypted result to client
            int dec_size = strlen(dec_buffer);
            if (send(connect_socket, &dec_size, sizeof(dec_size), 0) < 0)
            {
                fprintf(stderr, "Server: Error Failed to send an initial message to Client\n");
                exit(2);
            }
            if (send(connect_socket, dec_buffer, dec_size, 0) < 0)
            {
                fprintf(stderr, "Server: Error Failed to send an initial message to Client\n");
                exit(2);
            }

            // Clean up and exit
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
