#ifndef HTTP_H_
#define HTTP_H_

#define REQUEST_SIZE 16384

typedef enum {
    GET,
    HEAD,
    POST
} method_t;

typedef struct header {
    char* name;
    char* value;
    struct header* next;
} header_t;


typedef struct {
    method_t method;
    char* target;
    char protocol[9];
    header_t* headers;
    char* body;
} request_t;

typedef struct {
    char protocol[9];
    int status;
    char* message;
    header_t* headers;
    char* body;
} response_t;


/* Gets the next string in the given message, stopping at delim. Unlike
strtok(), this does not "eat" multiple deliminators in a row, so empty strings
are returned for calls between deliminators.
Returns a null-terminated string.
message: the HTTP message to parse. Should not contain any null characters.
delim: the deliminator character to stop at.
size: the size of the message.
*/
char* get_token(char* message, char delim, int size);



#endif