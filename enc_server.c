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
char *input_buffer, *key_buffer, *enc_output_buffer;

void catch_child_signal(int signo){
    pid_t child_pid = 0;
    do {
        int child_ret;
        child_pid = waitpid(-1, &child_ret, WNOHANG);
    } while (child_pid > 0);
}

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
        fprintf(stderr, "Server: ERROR on binding\n");
        exit(2);
    }

    listen(listen_socket, 5);

    while (1)
    {
        connect_socket = accept(listen_socket, (struct sockaddr *)&client_address, &sizeOfClientInfo);
        if (connect_socket < 0)
        {
            fprintf(stderr, "Server: Error on accept\n");
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
            
            char recv_name[256];
            int reply;
            memset(recv_name, '\0', 256);
            
            // Receive input message and key message
            if (recv(connect_socket, recv_name, 255, 0) < 0)
            {
                fprintf(stderr, "Server: Error reading text_length from client\n");
                exit(2);
            }
            
            if (strcmp(recv_name, "enc_lient\0") == 0) 
            {
                reply = 1;
            } 
            else {
                reply = 0;
            }
            
            if (send(connect_socket, &reply, sizeof(reply), 0))
            {
                fprintf(stderr, "Server: Error failed to send message to client\n");
                exit(2);
            }
                
            input_buffer = (char *)calloc(text_length + 1, sizeof(char));
            memset(input_buffer, '\0', text_length + 1);

            // Receive input_buffer
            int total_received = 0, received, index = 0;
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

                received = recv(connect_socket, input_buffer + index, data_size, 0);
                if (received < 0)
                {
                    fprintf(stderr, "Server: Error reading input_buffer from client\n");
                    exit(2);
                }
                total_received += received;
                if (total_received >= text_length)
                {
                    is_done = 1;
                }
                index += received;
            }

            // Receive key_length
            if (recv(connect_socket, &key_length, sizeof(key_length), 0) < 0)
            {
                fprintf(stderr, "Server: Error reading key_length from client\n");
                exit(2);
            }

            key_buffer = (char *)calloc(key_length + 1, sizeof(char));
            memset(key_buffer, '\0', key_length + 1);

            // Receive key_buffer
            total_received = 0;
            index = 0;
            is_done = 0;

            while (!is_done)
            {
                if (key_length - total_received >= 5000)
                {
                    data_size = 5000;
                }
                else
                {
                    data_size = key_length - total_received;
                }

                received = recv(connect_socket, key_buffer + index, data_size, 0);
                if (received < 0)
                {
                    fprintf(stderr, "Server: Error reading key_buffer from client\n");
                    exit(2);
                }

                total_received += received;
                if (total_received >= key_length)
                {
                    is_done = 1;
                }
                index += received;
            }

            // Encrypt the message
            int text_size = strlen(input_buffer);
            enc_output_buffer = (char *)calloc(text_size + 1, sizeof(char));
            memset(enc_output_buffer, '\0', text_size + 1);

            for (int i = 0; i < text_size; ++i)
            {
                int text_index = strchr(key_chars, input_buffer[i]) - key_chars, key_index = strchr(key_chars, key_buffer[i]) - key_chars;
                enc_output_buffer[i] = key_chars[(text_index + key_index) % strlen(key_chars)];
            }

            // Send encrypted result to client
            int enc_size = strlen(enc_output_buffer);
            if (send(connect_socket, &enc_size, sizeof(enc_size), 0) < 0)
            {
                fprintf(stderr, "Server: Error sending encrypted size to client\n");
                exit(2);
            }

            if (send(connect_socket, enc_output_buffer, enc_size, 0) < 0)
            {
                fprintf(stderr, "Server: Error sending encrypted message to client\n");
                exit(2);
            }

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
