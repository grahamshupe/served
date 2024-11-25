#ifndef RESPONSE_H_
#define RESPONSE_H_

#include "request.h"

typedef struct response {
    char protocol[9];
    int status;
    char reason[32];
    struct header* headers;
    int body_fd;
    ssize_t bytes_sent;  // The total number of bytes of the response sent, including the body
    int msg_size;  // The size of the response, excluding the body
} response_t;

/*
Dynamically allocates a new response struct and initializes it with the given STATUS.
Basic headers (date and server info) will be added.
*/
response_t* resp_new(int status);

/*
Changes the status and reason fields of RESP, given STATUS.
*/
void resp_change_status(response_t* resp, int status);

/*
Adds a new header to RESP with the given NAME and VALUE.
*/
void resp_add_header(response_t* resp, const char* name, const char* value);

/*
Converts the given response into a string.
The string will be dynamically allocated and placed into STR.
Returns the length of the allocated string.
*/
int resp_to_str(response_t* resp, char** str);

/*
Frees all malloc'd members in RESP, including RESP itself.
*/
void resp_free(response_t* resp);

/*
Returns the value of the given header name in RESP.
*/
char* resp_get_header(response_t* resp, const char* name);


#endif