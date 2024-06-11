#ifndef RESPONSE_H_
#define RESPONSE_H_

#include "request.h"

#define BODY_SIZE 32768

struct response {
    char protocol[9];
    int status;
    char reason[32];
    struct header* headers;
    char* body;
};

/*
Fills the members of the given HTTP response.
Basic headers (date and server info) will be added.
*/
void resp_new(struct response* resp);

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
*/
int resp_to_str(struct response* resp, char* str);

/*
Frees all malloc'd members in RESP.
*/
void resp_free(struct response* resp);

/*
Fill the given RESP as a response to a http GET request.
*/
int handle_get(struct response* resp, struct request* req, 
                const char* root_path);

/*
Fill the given RESP as a response to a http POST request.
*/
int handle_post(struct response* resp, struct request* req);

#endif