#ifndef HANDLER_H_
#define HANDLER_H_

//#include "server.h"
#include "request.h"
#include "response.h"

typedef enum {
    ERR = -1,  // Connection has caused a server error
    IDLE,  // Waiting for a request
    READING,  // Has read some, but not all of a request
    WRITING  // Has sent some, but not all of a response
} conn_state_t;

typedef struct {
    int fd;
    conn_state_t state;
    request_t* req;
    response_t* resp;
    char* msg_buffer;
    ssize_t buffer_size;
} conn_info_t;

/*
Handles a read (EPOLLIN) event on the given conn_info fd.
Any pending requests will be parsed, and a response will be created.
This will attempt to send all pending responses, using handle_write_event().
Returns -1 on error, 0 if the socket has shut down, and 1 on success.
*/
int handle_read_event(conn_info_t* conn_info, char* rootpath);

/*
Handles a write (EPOLLOUT) event on the given conn_info fd.
This constructs and sends a response to the pending request.
On success, the connection state is returned to IDLE, and req and resp are freed.
Returns -1 on error, 0 if the socket has shut down, and 1 on success.
*/
int handle_write_event(conn_info_t* conn_info, char* rootpath);


#endif