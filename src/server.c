#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define PORT "80"
#define QUEUE_SIZE 32
#define REQUEST_SIZE 16384  // 16K


// Gets a listening socket for the server.
int get_socket() {
    struct addrinfo hints;
    struct addrinfo* addrlist;

    // Set up hints:
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
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

        // ** TODO: WILL THIS CAUSE PROBLEMS FOR REAL WEB SERVER? **
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

        // All done
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
void handle_connection(int client_fd) {
    char* request = malloc(REQUEST_SIZE);
    int received = recv(client_fd, request, REQUEST_SIZE - 1, 0);
    
    if (received == -1) {
        perror("recv");
        free(request);
        return;
    }

    
    
}


int main() {
    puts("Served: setting up listening socket");
    int sock_fd = get_socket();

    printf("Served: waiting for connections on port %s...\n", PORT);
    while (1) {
        // TODO: print out client address given by accept()
        int client_fd = accept(sock_fd, NULL, NULL);

        // TODO: make this multi-threaded
        handle_connection(client_fd);

        close(client_fd);
    }


    return 0;
}