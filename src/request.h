#ifndef REQUEST_H_
#define REQUEST_H_

#define REQUEST_SIZE 4096  // Max total size of the request, excluding the body
#define MAX_TARGET_SIZE 512  // Max size of the request-line target

typedef enum {
    GET,
    HEAD,
    POST
} method_t;

typedef struct request {
    method_t method;
    char* target;
    char* protocol;
    struct header* headers;
    char* body;
    char* raw_message;
    ssize_t msg_size;
    ssize_t bytes_read;  // The number of bytes from raw_message that have been processed
} request_t;

/*
Parses a HTTP message into a HTTP request.
If REQ is null, a new struct will be malloc'd and returned, otherwise REQ must be a partially
complete or zero'd out struct, which will also be returned.
STATUS will be filled with a HTTP status code representing whether parsing was successful,
or 0 if the message is grammatically correct but incomplete.
*/
request_t* req_parse(char* message, request_t* req, ssize_t msg_size, int* status);

/*
Free all malloc'd members in REQ, including REQ itself
*/
void req_free(struct request* req);

/*
Returns the value of the header in REQ with the given NAME,
or NULL if not found.
*/
char* req_get_header(struct request* req, const char* name);


#endif