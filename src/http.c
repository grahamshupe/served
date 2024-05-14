/*
Functions for creating and modifying HTTP requests and responses.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "http.h"


#define TARGET_SIZE 1024


int get_token(const char* message, char delim, int size) {
    int i = 0;
    while (message[i] != delim && i < size) {
        i++;
    }
    return i;
}


int req_parse_start(char* message, request_t* request) {
    char* msg_ptr = message;
    // Get method:
    if (strncmp(message, "GET ", 4)) {
        request->method = GET;
        msg_ptr + 4;
    } else if (strncmp(message, "HEAD ", 5)) {
        request->method = HEAD;
        msg_ptr + 5;
    } else if (strncmp(message, "POST ", 5)) {
        request->method = POST;
        msg_ptr + 5;
    } else {
        // invalid method
        return 501;
    }

    // Get target:
    int token_size = get_token(msg_ptr, ' ', TARGET_SIZE);
    if (token_size == 0) {
        // no target or too many spaces given
        return 400;
    } else if (token_size == TARGET_SIZE) {
        // target is too long
        return 414;
    }
    request->target = (char*) malloc(token_size + 1);
    strncpy(request->target, msg_ptr, token_size);
    request->target[token_size] = '\0';

    // Get protocol:
    msg_ptr += token_size + 1;
    if (strncmp(msg_ptr, "HTTP/1.1\r\n", 10) != 0) {
        // invalid protocol (must be HTTP/1.1)
        free(request->target);
        request->target = NULL;
        return 400;
    }
    strncpy(request->protocol, "HTTP/1.1", 9);

    return 200;
}

int req_parse_headers(char* message, request_t* request) {

}

int req_parse(char* message, request_t* request) {
    int status = 0;
    status = req_parse_start(message, request);
    if (status != 200) {
        return status;
    }
    status = req_parse_headers(message, request);
    if (status != 200) {
        return status;
    }

}

void req_free() {

}