/**
 * Charming Chatroom
 * CS 241 - Fall 2019
 * 
 * Advai Podduturi, advairp2
 * Adi Budithi, budithi2
 * Robert Lou, robertl3
 */
#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "utils.h"

#define MAX_CLIENTS 8

void *process_client(void *p);

static volatile int serverSocket;
static volatile int endSession;

static volatile int clientsCount;
static volatile int clients[MAX_CLIENTS];

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

/**
 * Signal handler for SIGINT.
 * Used to set flag to end server.
 */
void close_server() {
    endSession = 1;
    // add any additional flags here you want.
}

/**
 * Cleanup function called in main after run_server exits.
 * Server ending clean up (such as shutting down clients) should be handled
 * here.
 */
void cleanup() {
    if (shutdown(serverSocket, SHUT_RDWR) != 0) {
        perror("shutdown():");
    }
    close(serverSocket);

    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i] != -1) {
            if (shutdown(clients[i], SHUT_RDWR) != 0) {
                perror("shutdown(): ");
            }
            close(clients[i]);
        }
    }
}

/**
 * Sets up a server connection.
 * Does not accept more than MAX_CLIENTS connections.  If more than MAX_CLIENTS
 * clients attempts to connects, simply shuts down
 * the new client and continues accepting.
 * Per client, a thread should be created and 'process_client' should handle
 * that client.
 * Makes use of 'endSession', 'clientsCount', 'client', and 'mutex'.
 *
 * port - port server will run on.
 *
 * If any networking call fails, the appropriate error is printed and the
 * function calls exit(1):
 *    - fprtinf to stderr for getaddrinfo
 *    - perror() for any other call
 */
void run_server(char *port) {
    int socket_id = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_id == -1) {
        perror(NULL);
        exit(1);
    }
    struct addrinfo hints, *head;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    int getaddrinfo_result = getaddrinfo(NULL, port, &hints, &head);
    if (getaddrinfo_result != 0) {
        const char *error_message = gai_strerror(getaddrinfo_result);
        fprintf(stderr, "%s", error_message);
        if (head) freeaddrinfo(head);
        close_server();
        exit(1);
    }

    int optval = 1;
    int setsockopt_result = setsockopt(socket_id, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));
    if (setsockopt_result == -1) {
        perror(NULL);
        if (head) freeaddrinfo(head);
        close_server();
        exit(1);
    }

    int bind_result = bind(socket_id, head->ai_addr, head->ai_addrlen);
    if (bind_result == -1) {
        perror(NULL);
        if (head) freeaddrinfo(head);
        close_server();
        exit(1);
    }
    
    int listen_result = listen(socket_id, MAX_CLIENTS);
    if (listen_result == -1) {
        perror(NULL);
        if (head) freeaddrinfo(head);
        close_server();
        exit(1);
    }

    for (int i = 0; i < MAX_CLIENTS; i++) {
        clients[i] = -1;
    }

    while (endSession == 0) {
        if (clientsCount < MAX_CLIENTS) {
            struct sockaddr client;
            socklen_t client_size = sizeof(client);
            memset(&client, 0, client_size);
            printf("waiting for user to join\n");
            int client_id = accept(socket_id, (struct sockaddr*) &client, &client_size);
            printf("before end session\n");
            if (endSession != 0) {
                break;
            }
            printf("before client_id check\n");
            if (client_id == -1) {
                perror(NULL);
                if (head) freeaddrinfo(head);
                close_server();
                exit(1);
            }

            intptr_t client_idx = -1;
            pthread_mutex_lock(&mutex);
            printf("locked\n");
            for (int i = 0; i < MAX_CLIENTS; i++) {
                printf("looking for client\n");
                if (clients[i] == -1) {
                    clients[i] = client_id;
                    client_idx = i;
                    printf("found free client\n");
                    break;
                }
            }
            clientsCount++;
            printf("user joined\n");
            pthread_mutex_unlock(&mutex);

            pthread_t client_thread;
            int pthread_result = pthread_create(&client_thread, NULL, process_client, (void*)client_idx);
            if (pthread_result == -1) {
                perror(NULL);
                if (head) freeaddrinfo(head);
                close_server();
                exit(1);
            }
        }
    }
    if (serverSocket != -1) {
        close_server();
    }
}

/**
 * Broadcasts the message to all connected clients.
 *
 * message  - the message to send to all clients.
 * size     - length in bytes of message to send.
 */
void write_to_clients(const char *message, size_t size) {
    pthread_mutex_lock(&mutex);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i] != -1) {
            ssize_t retval = write_message_size(size, clients[i]);
            if (retval > 0) {
                retval = write_all_to_socket(clients[i], message, size);
            }
            if (retval == -1) {
                perror("write(): ");
            }
        }
    }
    pthread_mutex_unlock(&mutex);
}

/**
 * Handles the reading to and writing from clients.
 *
 * p  - (void*)intptr_t index where clients[(intptr_t)p] is the file descriptor
 * for this client.
 *
 * Return value not used.
 */
void *process_client(void *p) {
    pthread_detach(pthread_self());
    intptr_t clientId = (intptr_t)p;
    ssize_t retval = 1;
    char *buffer = NULL;

    while (retval > 0 && endSession == 0) {
        retval = get_message_size(clients[clientId]);
        if (retval > 0) {
            buffer = calloc(1, retval);
            retval = read_all_from_socket(clients[clientId], buffer, retval);
        }
        if (retval > 0)
            write_to_clients(buffer, retval);

        free(buffer);
        buffer = NULL;
    }

    printf("User %d left\n", (int)clientId);
    close(clients[clientId]);

    pthread_mutex_lock(&mutex);
    clients[clientId] = -1;
    clientsCount--;
    pthread_mutex_unlock(&mutex);

    return NULL;
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "%s <port>\n", argv[0]);
        return -1;
    }

    struct sigaction act;
    memset(&act, '\0', sizeof(act));
    act.sa_handler = close_server;
    if (sigaction(SIGINT, &act, NULL) < 0) {
        perror("sigaction");
        return 1;
    }

    run_server(argv[1]);
    cleanup();
    pthread_exit(NULL);
}