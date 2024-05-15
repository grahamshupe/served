#ifndef HTTP_H_
#define HTTP_H_

#define REQUEST_SIZE 16384

typedef enum {
    GET,
    HEAD,
    POST
} method_t;

struct header {
    char* name;
    char* value;
    struct header* next;
};


struct request {
    method_t method;
    char* target;
    char protocol[9];
    struct header* headers;
    char* body;
};

struct response {
    char protocol[9];
    int status;
    char* message;
    struct header* headers;
    char* body;
};


/* Gets the length of message to the first occurence of delim.
Unlike strcspn(), this only reads a max of SIZE bytes.
*/
int get_token(const char* message, char delim, int size);

/* Parses the request-line of the given message.
Returns a HTTP status code representing success or failure.
MESSAGE will be moved to the end of the startline.
*/
int req_parse_start(char* message, struct request* req);


/* Creates a new HTTP request from the given HTTP message.
On success, request is filled and 200 is returned.
On failure, the HTTP status code of the error is returned.
*/
int req_parse(char* message, struct request* req);


#endif