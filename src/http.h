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
    char reason[32];
    struct header* headers;
    char* body;
};


/*
Gets the length of STR to the first occurence of any character in REJECT.
Unlike strcspn(), this only reads a max of SIZE bytes.
*/
size_t memcspn(const char* str, const char* reject, size_t str_size);

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
Fills the members of the given HTTP response.
Generic headers will be added. Additional headers must be added after
calling this function.
*/
void resp_new(struct response* resp, int status, char* body);

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

#endif