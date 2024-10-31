#ifndef RESPONSE_H_
#define RESPONSE_H_

#include "request.h"

typedef struct response {
    char protocol[9];
    int status;
    char reason[32];
    struct header* headers;
} response_t;

/*
Creates a new response struct and initializes it with the given STATUS.
Basic headers (date and server info) will be added.
*/
response_t* resp_new(int status);

/*
Changes the status and reason fields of RESP, given STATUS.
*/
void resp_change_status(struct response* resp, int status);

/*
Adds a new header to RESP with the given NAME and VALUE.
*/
void resp_add_header(struct response* resp, const char* name, 
                    const char* value);

/*
Converts the given response into a string, ready to be sent as a HTTP message.
The body must be sent separately, preferably using sendfile(2).
*/
int resp_to_str(struct response* resp, char* str);

/*
Frees all malloc'd members in RESP, including RESP itself.
*/
void resp_free(struct response* resp);



#endif