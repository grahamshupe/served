#ifndef REQUEST_H_
#define REQUEST_H_

#define REQUEST_SIZE 16384

typedef enum {
    GET,
    HEAD,
    POST
} method_t;

struct request {
    method_t method;
    char* target;
    char protocol[9];
    struct header* headers;
    char* body;
};

/*
Parses a HTTP message into a HTTP request.
On success, REQ is filled and 200 is returned.
On failure, the HTTP status code of the error is returned.
*/
int req_parse(char* message,  struct request* req, size_t msg_size);

/*
Free all malloc'd members in REQ.
*/
void req_free(struct request* req);

/*
Returns the value of the header in REQ with the given NAME,
or NULL if not found.
*/
char* req_get_header(struct request* req, const char* name);


#endif