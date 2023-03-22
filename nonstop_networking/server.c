/**
 * Nonstop Networking
 * CS 241 - Fall 2019
 */

#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/epoll.h>
#include <unistd.h>
#include "common.h"
#include "format.h"
#include "vector.h"
#include "dictionary.h"

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define MAX_CLIENTS 1024

typedef struct info {
    int cfd;
    int fd;
    verb cmd;
    char* file_name;
    size_t file_size;
} info;

static volatile int sfd;
static volatile int epfd;
static vector* files;
static char* directory;
static char template[] = "./XXXXXX";
static struct epoll_event curr_event;
static int go = 1;
static struct addrinfo* head;

int start_socket(char* port);
void run_server();
void close_server();
struct info* get_info();
void destroy_info(struct info* stuff);
int process_header();
int get_cmd();
ssize_t get_message_size();
char* get_file_name();
char* get_file_path(char* file_name);
size_t read_all_from_socket(int socket, char *buffer, size_t count);
size_t write_all_to_socket(int socket, const char *buffer, size_t count);
ssize_t read_from_file(int fd, char *buffer, size_t count);
ssize_t write_to_file(int fd, const char *buffer, size_t count);
int list();
int delete();
int get();
int put();
void yeet();

int main(int argc, char **argv) {
    // good luck!
    if (argc != 2) {
        print_server_usage();
        return -1;
    }

    struct sigaction action;
    memset(&action, '\0', sizeof(action));
    action.sa_handler = close_server;
    if (sigaction(SIGINT, &action, NULL) < 0) {
        perror("sigaction");
        return 1;
    }

    struct sigaction pipe;
    memset(&pipe, '\0', sizeof(pipe));
    pipe.sa_handler = yeet;
    if (sigaction(SIGPIPE, &pipe, NULL) < 0) {
        perror("sigaction");
        return 1;
    }

    files = vector_create(string_copy_constructor, string_destructor, string_default_constructor);
    directory = mkdtemp(template);
    
    sfd = start_socket(argv[1]);
    epfd = epoll_create1(0);

    run_server();

    return 0;
}

/*
 * TAKEN FROM LAB
 */
int start_socket(char* port) {
    int socket_id = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_id == -1) {
        perror(NULL);
        exit(1);
    }

    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    int getaddrinfo_result = getaddrinfo(NULL, port, &hints, &head);
    if (getaddrinfo_result != 0) {
        const char *error_message = gai_strerror(getaddrinfo_result);
        fprintf(stderr, "%s", error_message);
        close_server();
        exit(1);
    }

    int optval = 1;
    int setsockopt_result = setsockopt(socket_id, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &optval, sizeof(optval));
    if (setsockopt_result == -1) {
        perror(NULL);
        close_server();
        exit(1);
    }

    int bind_result = bind(socket_id, head->ai_addr, head->ai_addrlen);
    if (bind_result == -1) {
        perror(NULL);
        close_server();
        exit(1);
    }
    
    int listen_result = listen(socket_id, MAX_CLIENTS);
    if (listen_result == -1) {
        perror(NULL);
        close_server();
        exit(1);
    }

    printf("socket connected...\n");

    return socket_id;
}

void run_server() {
    printf("starting server...\n");

    while (go) {
        int cfd = accept(sfd, NULL, NULL);

        if (cfd == -1) continue;

        struct info* stuff;
        struct epoll_event ev;

        printf("accepted client %d...\n", cfd);

        memset(&ev, 0, sizeof(ev));
        stuff = calloc(1, sizeof(struct info));
        stuff->cfd = cfd;
        ev.events = EPOLLIN | EPOLLOUT;
        ev.data.ptr = (void*) stuff;

        epoll_ctl(epfd, EPOLL_CTL_ADD, cfd, &ev);
        if (epoll_wait(epfd, &curr_event, 1, 1) == 0) continue;

        stuff = get_info();

        if (process_header() == 0) {
            epoll_ctl(epfd, EPOLL_CTL_DEL, stuff->cfd, &curr_event);
            write_all_to_socket(stuff->cfd, err_bad_request, strlen(err_no_such_file));
            destroy_info(stuff);
            continue;
        }

        if (stuff->cmd == LIST) {
            list();
            close(stuff->cfd);
            epoll_ctl(epfd, EPOLL_CTL_DEL, stuff->cfd, &curr_event);
            destroy_info(stuff);
            continue;
        } else if (stuff->cmd == DELETE) {
            delete();
            close(stuff->cfd);
            epoll_ctl(epfd, EPOLL_CTL_DEL, stuff->cfd, &curr_event);
            destroy_info(stuff);
            continue;
        } else if (stuff->cmd == GET) {
            get();
            close(stuff->cfd);
            epoll_ctl(epfd, EPOLL_CTL_DEL, stuff->cfd, &curr_event);
            destroy_info(stuff);
            continue;
        } else if (stuff->cmd == PUT) {
            put();
            close(stuff->cfd);
            epoll_ctl(epfd, EPOLL_CTL_DEL, stuff->cfd, &curr_event);
            destroy_info(stuff);
            continue;
        }

        destroy_info(stuff);

        break;
    }
}

void close_server() {
    go = 0;

    VECTOR_FOR_EACH(files, file_name, {
        char* path = get_file_path(file_name);
        unlink(path);
        free(path);
    });

    vector_destroy(files);
    rmdir(directory);
    freeaddrinfo(head);
    close(sfd);

    printf("server closed\n");
}

struct info* get_info() {
    return (struct info*) curr_event.data.ptr;
}

void destroy_info(struct info* stuff) {
    free(stuff->file_name);
    free(stuff);
}

int process_header() {
    printf("processing header...\n");
    struct info* stuff = get_info();

    if (get_cmd() == 0) {
        print_invalid_response();
        return 0;
    }

    stuff->file_name = get_file_name();

    if (stuff->file_name == NULL) {
        return 0;
    }

    if (stuff->cmd == PUT) {
        stuff->file_size = get_message_size();
    } else {
        stuff->file_size = 0;
    }

    return 1;
}

int get_cmd() {
    printf("reading command...\n");

    struct info* stuff = get_info(); 
    char* buf = calloc(10, sizeof(char));
    read_all_from_socket(stuff->cfd, buf, 3);

    if (!strcmp(buf, "PUT")) {
        read_all_from_socket(stuff->cfd, buf, 1);
        free(buf);
        stuff->cmd = PUT;
        return 1;
    } else if (!strcmp(buf, "GET")) {
        read_all_from_socket(stuff->cfd, buf, 1);
        free(buf);
        stuff->cmd = GET;
        return 1;
    } else if (!strcmp(buf, "DEL")) {
        read_all_from_socket(stuff->cfd, buf, 4);
        free(buf);
        stuff->cmd = DELETE;
        return 1;
    } else if (!strcmp(buf, "LIS")) {
        free(buf);
        stuff->cmd = LIST;
        return 1;
    }

    free(buf);

    return 0;
}

/**
 * TAKEN FROM CHARMING CHATROOM LAB
 */
ssize_t get_message_size() {
    struct info* stuff = get_info();
    size_t size;
    ssize_t read_bytes = read_all_from_socket(stuff->cfd, (char *) &size, sizeof(size_t));
    if (read_bytes == 0 || read_bytes == -1) {
        return read_bytes;
    }
    return size;
}

char* get_file_name() {
    struct info* stuff = get_info();
    char* buf = calloc(256, sizeof(char));

    for (int i = 0; i < 256; i++) {
        read_all_from_socket(stuff->cfd, buf + i, 1);
        if (!strcmp(buf + i, "\n")) {
            buf[i] = '\0';
            return buf;
        }
    }

    free(buf);
    return NULL;
}

char* get_file_path(char* file_name) {
    char* path = NULL;
    asprintf(&path, "%s/%s", directory, file_name);
    return path;
}

/**
 * TAKEN FROM CHARMING CHATROOM LAB
 */
size_t read_all_from_socket(int socket, char *buffer, size_t count) {
    size_t bytes_read = 0;
    while (bytes_read < count) {
        int read_retval = read(socket, (void*) (buffer + bytes_read), count - bytes_read);
        if (read_retval == 0) {
            return 0;
        } else if (read_retval == -1 && errno == EINTR) {
            continue;
        } else if (read_retval > 0) {
            bytes_read += read_retval;
        } else {
            return -1;
        }
    }
    return bytes_read;
}

/*
 * TAKEN FROM CHARMING CHATROOM LAB
 */
size_t write_all_to_socket(int socket, const char *buffer, size_t count) {
    size_t bytes_written = 0;

    while (bytes_written < count) {
        int write_retval = write(socket, (void*) (buffer + bytes_written), count - bytes_written);
        if (write_retval == 0) {
            return 0;
        } else if (write_retval == -1 && errno == EINTR) {
            continue;
        } else if (write_retval > 0) {
            bytes_written += write_retval;
        } else {
            return -1;
        }
    }

    return bytes_written;
}

ssize_t read_from_file(int fd, char *buffer, size_t count) {
    ssize_t bytes_read = 0;
    ssize_t bytes_left = count;
    ssize_t read_retval;
    while (bytes_left > 0 || (read_retval == -1 && errno == EINTR)) {
        read_retval = read(fd, (void*) (buffer + bytes_read), bytes_left);
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

ssize_t write_to_file(int fd, const char *buffer, size_t count) {
    // Your Code Here
    ssize_t bytes_written = 0;
    ssize_t bytes_left = count;
    ssize_t write_retval;
    while (bytes_left > 0 || (write_retval == -1 && errno == EINTR)) {
        write_retval = write(fd, (void*) (buffer + bytes_written), bytes_left);
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

int list() {
    printf("LIST acknowledged...\n");
    struct info* stuff = get_info();
    write_all_to_socket(stuff->cfd, "OK\n", strlen("OK\n"));
    size_t list_size = 0;

    VECTOR_FOR_EACH(files, file_name, {
        list_size += strlen(file_name) + 1;
    });

    list_size--;

    if (vector_size(files) == 0) {
        list_size = 0;
    }

    write_all_to_socket(stuff->cfd, (char*) &list_size, sizeof(size_t));
    printf("list size = %zd\n", list_size);

    size_t i = MAX(vector_size(files) - 1, 0);
    VECTOR_FOR_EACH(files, file_name, {
        write_all_to_socket(stuff->cfd, file_name, strlen(file_name));
        if (i > 0) write_all_to_socket(stuff->cfd, "\n", strlen("\n"));
        i--;
    });

    printf("LIST completed...\n");

    return 1;
}

int delete() {
    printf("DELETE acknowledged...\n");
    struct info* stuff = get_info();
    size_t i = 0;

    VECTOR_FOR_EACH(files, file_name, {
        if (!strcmp(stuff->file_name, file_name)) {
            printf("%s found...\n", file_name);

            write_all_to_socket(stuff->cfd, "OK\n", strlen("OK\n"));
            char* path = get_file_path(stuff->file_name);
            remove(path);
            vector_erase(files, i);
            free(path);

            printf("%s deleted...\n", file_name);

            return 1;
        }
        i++;
    });

    write_all_to_socket(stuff->cfd, "ERROR\n", strlen("ERROR\n"));
    write_all_to_socket(stuff->cfd, err_no_such_file, strlen(err_no_such_file));

    printf("DELETE completed...\n");

    return 0;
}

int get() {
    printf("GET acknowledged...\n");
    struct info* stuff = get_info();
    char* path = NULL;

    VECTOR_FOR_EACH(files, file_name, {
        if (!strcmp(stuff->file_name, file_name)) {
            printf("%s found...\n", file_name);

            path = get_file_path(stuff->file_name);
            break;
        }
    });

    if (path == NULL) {
        printf("%s not found...\n", stuff->file_name);
        printf("GET completed...\n");
        return 0;
    }

    stuff->fd = open(path, O_RDONLY);

    struct stat stats;
    stat(path, &stats);

    stuff->file_size = (size_t) stats.st_size;

    printf("path = %s\n", path);

    printf("sending %zd bytes of data...\n", stuff->file_size);

    write_all_to_socket(stuff->cfd, "OK\n", strlen("OK\n"));
    write_all_to_socket(stuff->cfd, (char*) &(stuff->file_size), sizeof(size_t));

    char buf[1024];
    size_t bytes_sent = 0;
    size_t bytes_left = stuff->file_size;

    while (bytes_sent < stuff->file_size) {
        memset(buf, 0, sizeof(buf));
        read_from_file(stuff->fd, buf, MIN(sizeof(buf), bytes_left));
        size_t retval = write_all_to_socket(stuff->cfd, buf, MIN(sizeof(buf), bytes_left));

        bytes_sent += retval;
        bytes_left -= retval;
    }

    close(stuff->fd);
    free(path);

    printf("GET completed...\n");

    return 1;
}

int put() {
    printf("PUT acknowledged...\n");
    struct info* stuff = get_info();
    char* path = get_file_path(stuff->file_name);
    stuff->fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);

    char buf[1024];
    size_t bytes_read = 0;
    size_t bytes_left = stuff->file_size;

    printf("%zd bytes expected\n", bytes_left);

    while (bytes_read < stuff->file_size && bytes_left > 0) {
        memset(buf, 0, sizeof(buf));
        size_t retval = read_all_from_socket(stuff->cfd, buf, MIN(sizeof(buf), bytes_left));

        if (retval == 0) {
            if (bytes_read < stuff->file_size) {
                printf("not enough information given...\n");

                write_all_to_socket(stuff->cfd, "ERROR\n", strlen("ERROR\n"));
                write_all_to_socket(stuff->cfd, err_bad_file_size, strlen(err_bad_file_size));
                remove(path);
                return 0;
            }
        }

        write_to_file(stuff->fd, buf, retval);

        bytes_read += retval;
        bytes_left -= retval;
    }

    char next[1];
    int actual = read_all_from_socket(stuff->cfd, next, 1);

    if (actual != 0) {
        printf("too much information received...\n");

        write_all_to_socket(stuff->cfd, "ERROR\n", strlen("ERROR\n"));
        write_all_to_socket(stuff->cfd, err_bad_file_size, strlen(err_bad_file_size));
        close(stuff->fd);
        remove(path);
        return 0;
    }

    printf("%s created...\n", stuff->file_name);

    vector_push_back(files, stuff->file_name);
    free(path);

    printf("PUT completed...\n");

    return 1;
}

void yeet() {
    return;
}
