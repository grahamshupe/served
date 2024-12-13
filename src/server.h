#ifndef SERVER_H_
#define SERVER_H_

#include "request.h"
#include "response.h"

extern char* root_path;  // The base directory to retrieve files from

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




#endif