#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <pthread.h>
#include <errno.h>

//#include "server.h"
#include "response.h"
#include "request.h"
#include "handler.h"
#include "util.h"
#include "zf_log.h"

#define PORT "9015"
#define QUEUE_SIZE 64
#define DEFAULT_THREAD_NUM 4
#define MAX_EVENTS 64

char* rootpath = NULL;

struct threadargs {
    int epoll_fd;
    int listener_fd;
};

/*
Sets the given file descriptor, fd, to nonblocking
*/
void set_nonblocking(int fd) {
    int flags;
    if ((flags = fcntl(fd, F_GETFL)) == -1) {
        perror("fcntl");
        exit(EXIT_FAILURE);
    }
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        perror("fcntl");
        exit(EXIT_FAILURE);
    }
}

/*
Gets a listening socket for the server, returning its file descriptor.
*/
int get_socket() {
    struct addrinfo hints;
    struct addrinfo* addrlist;

    // Set up hints:
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    // Get a list of available interfaces:
    int status = getaddrinfo(NULL, PORT, &hints, &addrlist);
    if (status != 0) {
        fprintf(stderr, "getaddrinfo ERROR: %s\n", gai_strerror(status));
    }

    // Try to bind to a socket in addrlist:
    struct addrinfo* addr;
    int sock_fd;
    for (addr = addrlist; addr != NULL; addr = addr->ai_next) {
        // Try to make a socket:
        sock_fd = socket(addr->ai_family, addr->ai_socktype,
            addr->ai_protocol);
        if (sock_fd == -1) {
            // address doesn't work, try another
            continue;
        }

        // Prevent "address already in use" error for testing:
        int p;
        int success = 0;
        success = setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &p, sizeof(p));
        if (success == -1) {
            perror("setsockopt");
            close(sock_fd);
            freeaddrinfo(addrlist);
            return -1;
        }

        // Try to bind to socket:
        success = bind(sock_fd, addr->ai_addr, addr->ai_addrlen);
        if (success == -1) {
            close(sock_fd);
            continue;
        }

        break;
    }

    freeaddrinfo(addrlist);

    // Make sure we found a socket:
    if (addr == NULL) {
        fprintf(stderr, "ERROR: failed to find local address");
        return -1;
    }

    // Start listening on socket:
    if (listen(sock_fd, QUEUE_SIZE) == -1) {
        perror("listen");
        close(sock_fd);
        return -1;
    }

    set_nonblocking(sock_fd);

    return sock_fd;
}

/*
Listens to the given epoll instance.
*/
void* epoll_listen(struct threadargs* targs) {
    struct epoll_event* events = malloc(sizeof(struct epoll_event) * MAX_EVENTS);
    int epoll_fd = targs->epoll_fd;
    int listener_fd = targs->listener_fd;
    struct epoll_event ev;

    int nfds = 0;
    while (1) {
        nfds = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        if (nfds == -1) {
            perror("epoll_wait");
            exit(EXIT_FAILURE);
        }

        int conn_fd;
        for (int n = 0; n < nfds; n++) {
            if (events[n].data.fd == listener_fd) {
                // This is a new connection request on the main socket
                if ((conn_fd = accept(listener_fd, NULL, NULL)) == -1) {
                    perror("accept");
                    exit(EXIT_FAILURE);
                }
                set_nonblocking(conn_fd);

                conn_info_t* info = calloc(1, sizeof(conn_info_t));
                info->fd = conn_fd;

                ev.events = EPOLLIN | EPOLLET | EPOLLONESHOT;
                ev.data.ptr = info;

                if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, conn_fd, &ev) == -1) {
                    perror("epoll_ctl");
                    continue;
                }
            } else {
                // This is another request on an established connection
                conn_info_t* info = events[n].data.ptr;
                if (info->state == IDLE || info->state == READING) {
                    if (handle_read_event(info, rootpath) <= 0) {
                        ZF_LOGD("Closing connection on fd %d due to shutdown or error", info->fd);
                        close(info->fd);
                        free(info);
                        continue;
                    }
                } else if (info->state == WRITING) {
                    if (handle_write_event(info, rootpath) <= 0) {
                        ZF_LOGD("Closing connection on fd %d due to shutdown or error", info->fd);
                        close(info->fd);
                        free(info);
                        continue;
                    }
                }

                // Rearm the fd:
                if (info->state == IDLE || info->state == READING) {
                    ev.events = EPOLLIN | EPOLLET | EPOLLONESHOT;
                    ev.data.ptr = info;
                    if (epoll_ctl(epoll_fd, EPOLL_CTL_MOD, info->fd, &ev) == -1) {
                        ZF_LOGE("Could not modify epoll for fd %d: error %d", info->fd, errno);
                        continue;
                    }
                } else if (info->state == WRITING) {
                    ev.events = EPOLLOUT | EPOLLET | EPOLLONESHOT;
                    ev.data.ptr = info;
                    if (epoll_ctl(epoll_fd, EPOLL_CTL_MOD, info->fd, &ev) == -1) {
                        ZF_LOGE("Could not modify epoll for fd %d: error %d", info->fd, errno);
                        continue;
                    }
                } else {
                    ZF_LOGW("Uncaught error in fd %d, closing connection", info->fd);
                    close(info->fd);
                    free(info);
                    continue;
                }

            }
        }

    }
}


int main(int argc, char* argv[]) {
    int num_threads = DEFAULT_THREAD_NUM;  // todo: add this to optargs

    int opt = -1;
    while ((opt = getopt(argc, argv, "r:")) != -1) {
        switch (opt) {
            case 'r':
                rootpath = strdup(optarg);
                break;
            default:
                printf("Invalid argument '%c'\n", opt);
                return -1;
        }
    }

    if (rootpath == NULL)
        rootpath = strdup("test/root");

    puts("Served: setting up listening socket");
    int sock_fd = get_socket();

    int epoll_fd = epoll_create1(0);  // CLOSE THIS
    if (epoll_fd == -1) {
        perror("epoll_create1");
        return -1;
    }

    struct epoll_event epoll_ev;
    epoll_ev.events = EPOLLIN | EPOLLET;
    epoll_ev.data.fd = sock_fd;

    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, sock_fd, &epoll_ev) == -1) {
        perror("epoll_ctl");
        return -1;
    }

    pthread_t threads[num_threads];
    struct threadargs targs;
    targs.epoll_fd = epoll_fd;
    targs.listener_fd = sock_fd;
    for (int i = 0; i < num_threads; i++) {
        if(pthread_create(&threads[i], NULL, epoll_listen, &targs) == -1) {
            // error
        }
    }




    printf("Served: waiting for connections on port %s...\n", PORT);
    while (1) {
        int client_fd = accept(sock_fd, NULL, NULL);

        pid_t pid = 0;
        pid = fork();
        if (pid < 0) {
            perror("fork");
            return -1;
        }
        if (pid == 0) {
            // child:
            ZF_LOGD("Handling connection on socket %d", client_fd);

            //handle_connection(client_fd, root_path);
            close(client_fd);
            return 0;
        }
    }

    free(rootpath);  // unreachable but whatever
    return 0;
}
