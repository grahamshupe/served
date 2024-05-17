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


/*
Gets the length of STR to the first occurence of any character in REJECT.
Unlike strcspn(), this only reads a max of SIZE bytes.
*/
size_t memcspn(const char* str, const char* reject, size_t str_size);

/*
Creates a new HTTP request from the given HTTP message.
On success, REQ is filled and 200 is returned.
On failure, the HTTP status code of the error is returned.
*/
int req_parse(char* message,  struct request* req, size_t msg_size);

/*
Free all malloc'd members in REQ.
*/
void req_free(struct request* req);


#endif