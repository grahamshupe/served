#ifndef SERVER_H_
#define SERVER_H_

#include "request.h"

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
} conn_info_t;

typedef struct {
    char* root_path;
} server_opts_t;




#endif