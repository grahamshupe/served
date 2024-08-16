#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "response.h"
#include "request.h"
#include "util.h"
#include "zf_log.h"

#define PORT "9015"
#define QUEUE_SIZE 64
#define LOGFILE stdout


// Gets a listening socket for the server, returning its file descriptor.
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

    return sock_fd;
}

// Handles a http request from a client.
// client_fd must be an open socket to connect to.
void handle_connection(int client_fd, const char* root_path) {
    char* message = malloc(REQUEST_SIZE);
    char* msg_origin = message;  // keep track of start so we can free msg
    ssize_t received = recv(client_fd, message, REQUEST_SIZE, 0);

    if (received == -1) {
        perror("recv");
        free(message);
        return;
    }

    struct request req;
    int parse_status = req_parse(message, &req, received);
    struct response resp;
    resp_new(&resp);
    int resp_status = 500;

    if (parse_status != 200) {
        resp_status = parse_status;
    } else {
        switch (req.method) {
            case GET:
                resp_status = handle_get(&resp, &req, root_path);
                break;
            case POST:
                // right now, just deny all POST requests.
                // POST is functional, but since there is nothing to POST,
                // better to just deny it for security.
                resp_status = 403;
                //resp_status = handle_post(&resp, &req);
                break;
            case HEAD:
            default:
                resp_status = 501;
        }
    }

    resp_change_status(&resp, resp_status);
    char resp_msg[BODY_SIZE];
    int resp_size = resp_to_str(&resp, resp_msg);
    //printf("response:\n%s\n", resp_msg);
    int sent = send(client_fd, resp_msg, resp_size, 0);

    ZF_LOGI("Responded %d to connection attempt", resp.status);

    req_free(&req);
    resp_free(&resp);
    free(msg_origin);
    msg_origin, message = NULL;
}


int main(int argc, char* argv[]) {
    char* root_path = NULL;

    int opt = -1;
    while ((opt = getopt(argc, argv, "r:")) != -1) {
        switch (opt) {
            case 'r':
                root_path = strdup(optarg);
                break;
            default:
                printf("Invalid argument '%c'\n", opt);
                return -1;
        }
    }

    if (root_path == NULL)
        root_path = strdup("test/root");

    puts("Served: setting up listening socket");
    int sock_fd = get_socket();

    printf("Served: waiting for connections on port %s...\n", PORT);
    while (1) {
        // TODO: print out client address given by accept()
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

            handle_connection(client_fd, root_path);
            close(client_fd);
            return 0;
        }
    }

    free(root_path);  // unreachable but whatever
    return 0;
}
