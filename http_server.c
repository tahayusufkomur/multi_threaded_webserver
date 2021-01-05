#include <stdio.h>
#include <string.h> // for strlen
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h> // for inet_addr
#include <unistd.h>    // for write
#include <pthread.h>   // for threading, link with lpthread
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <semaphore.h>
#include "html_parser.h"

#define PATH "/Users/tahayusufkomur/Desktop/C/http_multithread_server/"                                                     // For example PATH cannot be "/home/ozgur/workspace/assignment2/sources/"
#define BUFFER_SIZE 1024
#define CONNECTION_NUMBER 10

int thread_count = 0; // Keeps the number of the threads working simultaneously.

typedef struct {

    pthread_t tid;
    char * msg;
} worker_thread;

void *tcp_handler(void *sock_desc, void *arg)
{
    int request;
    char client_reply[BUFFER_SIZE], *request_lines[3];
    char *size;

    // Get the socket descriptor.
    int sock = *((int *)sock_desc);

    // Get the request.
    request = recv(sock, client_reply, BUFFER_SIZE, 0);


    if (request < 0) // Receive failed.
    {
        printf("Recv failed\n");
    }
    if (request == 0) // receive socket closed. Client disconnected upexpectedly.
    {
        printf("Client disconnected upexpectedly.\n");
    }
    else // Message received.
    {   

        int size = GET_SIZE(client_reply);
        printf("%s", client_reply);
        if (!GET_CONTROL(client_reply) && !POST_CONTROL(client_reply) && !PUT_CONTROL(client_reply))
        {
            char * message = "HTTP/1.0 400 Bad Request\r\nContent-Type: text/html\r\n\r\n<!doctype html><html><body>You need to use GET request </body></html>";
            write(sock, message, strlen(message));
        }
        if (PUT_CONTROL(client_reply)>0)
        {
            char * message = "HTTP/1.0 501 Bad Request\r\nContent-Type: text/html\r\n\r\n<!doctype html><html><body> Not implemented </body></html>";
            printf("its PUT\n");
            write(sock, message, strlen(message));
        }
        if (POST_CONTROL(client_reply)>0)
        {
            char * message = "HTTP/1.0 501 Bad Request\r\nContent-Type: text/html\r\n\r\n<!doctype html><html><body> Not implemented </body></html>";
            printf("its POST\n");
            write(sock, message, strlen(message));
        }
       if (size < 100 || 20000 < size)
            {
                char * message = "HTTP/1.0 400 Bad Request\r\nContent-Type: text/html\r\n\r\n<!doctype html><html><body>You need to request file sized 100 between 20000.</body></html>";
                write(sock, message, strlen(message));
            }
        else 
        {

            char * mhead = "HTTP/1.0 200 OK\r\nContent-Type: text/html\r\n\r\n<!doctype html><html><body>";
            char * mend = "</body></html>";
            char * temp = (char*)malloc((size + strlen(mhead) + strlen(mend)) * sizeof(char));
            strcat(temp, mhead);
            for (int i = 0; i < size; i++)
            {
                temp = str_concatenator(temp, 'a', 2);
            }
            strcat(temp, mend);
            write(sock, temp, strlen(temp));

        }
    }
    free(sock_desc);
    shutdown(sock, SHUT_RDWR);
    close(sock);
    sock = -1;

}

int main(int argc, char *argv[])
{


    int sock_desc, new_socket, c, *new_sock;
    struct sockaddr_in server, client;
    //pthread_t worker_thread_ids[100];
    pthread_t tid;

    if (argc != 2)
    {
        printf("Correct usage is <http_multithread_server> <PORT_NO> \n");
        return 1;
    }
    int PORT_NO = atoi(argv[1]);
    sock_desc = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_desc == -1)
    {
        printf("Could not create socket\n");
        return 1;
    }

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORT_NO);

    if (bind(sock_desc, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        printf("Binding failed\n");
        return 1;
    }

    listen(sock_desc, 20);

    puts("Waiting for incoming connections...");
    c = sizeof(struct sockaddr_in);
    while ((new_socket = accept(sock_desc, (struct sockaddr *)&client, (socklen_t *)&c))) // Accept the connection.
    {


        printf("Connection accepted \n");
        new_sock = malloc(1);
        *new_sock = new_socket;
        if (pthread_create(&tid, NULL, (void *)tcp_handler, new_sock))
        {
            printf("Thread couldn't created \n");
        }

        pthread_join(tid, NULL);
    }



    return 0;
}