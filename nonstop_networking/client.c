/**
 * Nonstop Networking
 * CS 241 - Fall 2019
 */
#include "common.h"
#include "format.h"
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <fcntl.h>
#include <netdb.h>
#include <errno.h>

static volatile int serverSocket;
static char** args;
static char* host;
static char* port;
static char* remote;
static char* local;
static char* buf;

char **parse_args(int argc, char **argv);
verb check_args(char **args);
int connect_to_server(const char *host, const char *port);
ssize_t write_all_to_socket(const char *buffer, size_t count);
ssize_t read_all_from_socket(char *buffer, size_t count);
char* read_file(char* filename);
char* message(verb command, char* remote);
ssize_t get_message_size();
void close_client();
void check_error();
void delete();
void list();
void get();
void put();

int main(int argc, char **argv) {
    // Good luck!
    args = parse_args(argc, argv);
    verb command = check_args(args);

    host = args[0];
    port = args[1];
    remote = args[3];
    local = args[4];

    serverSocket = connect_to_server(host, port);
    buf = message(command, remote);

    if (command == DELETE) {  
        delete();
    } else if (command == LIST) {
        list();
    } else if (command == GET) {
        get();
    } else if (command == PUT) {
        put();
    }
    close_client();
    return 0;
}

/**
 * Given commandline argc and argv, parses argv.
 *
 * argc argc from main()
 * argv argv from main()
 *
 * Returns char* array in form of {host, port, method, remote, local, NULL}
 * where `method` is ALL CAPS
 */
char **parse_args(int argc, char **argv) {
    if (argc < 3) {
        return NULL;
    }

    char *host = strtok(argv[1], ":");
    char *port = strtok(NULL, ":");
    if (port == NULL) {
        return NULL;
    }

    char **args = calloc(1, 6 * sizeof(char *));
    args[0] = host;
    args[1] = port;
    args[2] = argv[2];
    char *temp = args[2];
    while (*temp) {
        *temp = toupper((unsigned char)*temp);
        temp++;
    }
    if (argc > 3) {
        args[3] = argv[3];
    }
    if (argc > 4) {
        args[4] = argv[4];
    }

    return args;
}

/**
 * Validates args to program.  If `args` are not valid, help information for the
 * program is printed.
 *
 * args     arguments to parse
 *
 * Returns a verb which corresponds to the request method
 */
verb check_args(char **args) {
    if (args == NULL) {
        print_client_usage();
        exit(1);
    }

    char *command = args[2];

    if (strcmp(command, "LIST") == 0) {
        return LIST;
    }

    if (strcmp(command, "GET") == 0) {
        if (args[3] != NULL && args[4] != NULL) {
            return GET;
        }
        print_client_help();
        exit(1);
    }

    if (strcmp(command, "DELETE") == 0) {
        if (args[3] != NULL) {
            return DELETE;
        }
        print_client_help();
        exit(1);
    }

    if (strcmp(command, "PUT") == 0) {
        if (args[3] == NULL || args[4] == NULL) {
            print_client_help();
            exit(1);
        }
        return PUT;
    }

    // Not a valid Method
    print_client_help();
    exit(1);
}

/**
 * TAKEN FROM LAB
 * Sets up a connection to a chatroom server and returns
 * the file descriptor associated with the connection.
 *
 * host - Server to connect to.
 * port - Port to connect to server on.
 *
 * Returns integer of valid file descriptor, or exit(1) on failure.
 */
int connect_to_server(const char *host, const char *port) {
    int socket_id = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_id == -1) {
        perror(NULL);
        exit(1);
    }
    struct addrinfo hints, *head;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    int getaddrinfo_result = getaddrinfo(host, port, &hints, &head);
    if (getaddrinfo_result != 0) {
        const char *error_message = gai_strerror(getaddrinfo_result);
        fprintf(stderr, "%s", error_message);
        if (head) freeaddrinfo(head);
        exit(1);
    }

    int connect_result = connect(socket_id, head->ai_addr, head->ai_addrlen);
    if (connect_result == -1) {
        perror(NULL);
        if (head) freeaddrinfo(head);
        exit(1);
    }

    if (head) freeaddrinfo(head);
    head = NULL;
    return socket_id;
}

/**
 * TAKEN FROM LAB
 */
ssize_t read_all_from_socket(char *buffer, size_t count) {
    // Your Code Here
    ssize_t bytes_read = 0;
    ssize_t bytes_left = count;
    ssize_t read_retval;
    while (bytes_left > 0 || (read_retval == -1 && errno == EINTR)) {
        read_retval = read(serverSocket, (void*) (buffer + bytes_read), bytes_left);
        if (read_retval == 0) {
            return 0;
        }
        if (read_retval > 0) {
            bytes_read += read_retval;
            bytes_left -= read_retval;
        }
    }
    return bytes_read;
}

/**
 * TAKEN FROM LAB
 */
ssize_t write_all_to_socket(const char *buffer, size_t count) {
    // Your Code Here
    ssize_t bytes_written = 0;
    ssize_t bytes_left = count;
    ssize_t write_retval;
    while (bytes_left > 0 || (write_retval == -1 && errno == EINTR)) {
        write_retval = write(serverSocket, (void*) (buffer + bytes_written), bytes_left);
        if (write_retval == 0) {
            return 0;
        }
        if (write_retval > 0) {
            bytes_written += write_retval;
            bytes_left -= write_retval;
        }
    }
    return count;
}

char* read_file(char* filename) {
    FILE* f = fopen(filename, "rb");
    char* buffer = NULL;
    long len;
    if (f) {
        fseek(f, 0, SEEK_END);
        len = ftell(f);
        fseek (f, 0, SEEK_SET);
        buffer = calloc(len, 1);
        if (buffer) {
            fread(buffer, 1, len, f);
        }
    }
    fclose(f);
    return buffer;
}

size_t file_size(char* filename) {
    FILE* f = fopen(filename, "rb");
    if (f) {
        fseek(f, 0L, SEEK_END);
        size_t size = (size_t) ftell(f);
        fclose(f);
        return size;
    }
    return 0;
}

char* message(verb command, char* remote) {
    char* msg = NULL;
    if (command == GET) {
        asprintf(&msg, "GET %s\n", remote);
    } else if (command == PUT) {
        asprintf(&msg, "PUT %s\n", remote);
    } else if (command == DELETE) {
        asprintf(&msg, "DELETE %s\n", remote);
    } else if (command == LIST) {
        asprintf(&msg, "LIST\n");
    }
    return msg;
}

/**
 * TAKEN FROM LAB
 */
ssize_t get_message_size() {
    size_t size;
    ssize_t read_bytes = read_all_from_socket((char *) &size, sizeof(size_t));
    if (read_bytes == 0 || read_bytes == -1) {
        return read_bytes;
    }
    return size;
}

void close_client() {
    if (args) free(args);
    if (buf) free(buf);
    exit(1);
}

void check_error() {
    read_all_from_socket(buf + 3, 3);
    printf("%s", buf);
    if (!strcmp(buf, "ERROR\n")) {
        read_all_from_socket(buf, 1024);
        print_error_message(buf);
    } else {
        print_invalid_response();
    }
}

void delete() {
    write_all_to_socket(buf, strlen(buf));
    shutdown(serverSocket, SHUT_WR);
    memset(buf, 0, strlen(buf));
    read_all_from_socket(buf, 3);
    if (!strcmp(buf, "OK\n")) {
        print_success();
    } else {
        check_error();
    }
}

void list() {
    write_all_to_socket(buf, strlen(buf));
    shutdown(serverSocket, SHUT_WR);
    memset(buf, 0, strlen(buf));
    read_all_from_socket(buf, 3);
    if (!strcmp(buf, "OK\n")) {
        ssize_t size = get_message_size();
        if (size) {
            buf = realloc(buf, sizeof(char) * size);
            read_all_from_socket(buf, size);
            printf("%s\n", buf);
        }
    } else {
        check_error();
    }
}

void get() {
    write_all_to_socket(buf, strlen(buf));
    shutdown(serverSocket, SHUT_WR);
    memset(buf, 0, strlen(buf));
    read_all_from_socket(buf, 3);
    if (!strcmp(buf, "OK\n")) {
        size_t size = get_message_size();
        if (buf) free(buf);
        buf = calloc(size + 1, sizeof(char));
        size_t actual = read_all_from_socket(buf, size);
        if (actual < size) {
            print_too_little_data();
        }
        char next[1];
        actual = read_all_from_socket(next, 1);
        if (actual != 0) {
            print_received_too_much_data();
        } else {
            FILE* f = fopen(local, "w+");
            fputs(buf, f);
            fclose(f);
        }
    } else {
        check_error();
    }
}

void put() {
    char* content = read_file(local);
    size_t size = file_size(local);
    if (content) {
        write_all_to_socket(buf, strlen(buf));
        write_all_to_socket((char *) &size, sizeof(size_t));
        write_all_to_socket(content, size);
        shutdown(serverSocket, SHUT_WR);
        memset(buf, 0, strlen(buf));
        read_all_from_socket(buf, 3);
        if (!strcmp(buf, "OK\n")) {
            print_success();
        } else {
            check_error();
        }
        free(content);
    }
}
