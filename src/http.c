
#include <stdio.h>
#include <stdlib.h>
#include "http.h"


char* get_token(char* message, char delim, int size) {
    for (int i = 0; message[i] != delim && i < size; i++) {
        
    }
    return "";
}


// Creates a new http request from the given http message.
// On success, status is set to 200 and the filled request
// is returned. On failure, status is set to the HTTP status
// code of the error and null is returned.
request_t* req_parse(const char* message, int* status) {
    char* request = (request_t*) malloc(REQUEST_SIZE);
    // Get start line tokens:
    char* token = malloc(64);
    for (int i = 0; message[i] != ' '; i++) {

    }
    for (char* c = message; *c != ' '; c++) {
        
    }
    

}

void req_free() {

}