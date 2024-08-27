/*
Functions for modifying and creating response structs.
*/

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "response.h"
#include "request.h"
#include "util.h"


void resp_new(struct response* resp) {
    strcpy(resp->protocol, "HTTP/1.1");
    resp->headers = NULL;
    resp->body = NULL;
    resp->status = 0;
    resp->reason[0] = '\0';

    // Add general headers:
    resp_add_header(resp, "Server", "Served");
    time_t timer = time(NULL);
    char* time = asctime(gmtime(&timer));
    time[24] = '\0';
    resp_add_header(resp, "Date", time);
    resp_add_header(resp, "Connection", "close");
}

void resp_change_status(struct response* resp, int status) {
    resp->status = status;
    switch (status) {
        // When adding new messages, make sure they are under 32 bytes!
        case 200:
            strcpy(resp->reason, "OK");
            break;
        case 303:
            strcpy(resp->reason, "See Other");
            break;
        case 400:
            strcpy(resp->reason, "Bad Request");
            break;
        case 404:
            strcpy(resp->reason, "Not Found");
            break;
        case 414:
            strcpy(resp->reason, "URI Too Long");
            break;
        case 500:
            strcpy(resp->reason, "Internal Server Error");
            break;
        case 501:
            strcpy(resp->reason, "Not Implemented");
            break;

        default:
            strcpy(resp->reason, "");
            break;
    }
}

void resp_add_header(struct response* resp, const char* name,
                    const char* value) {
    struct header* new = malloc(sizeof(struct header));

    new->name = malloc(strlen(name) + 1);
    strcpy(new->name, name);
    new->value = malloc(strlen(value) + 1);
    strcpy(new->value, value);

    new->next = resp->headers;
    resp->headers = new;
}

int resp_to_str(struct response* resp, char* str) {
    // Status line:
    int end = sprintf(str, "%s %i %s\r\n", resp->protocol, resp->status,
                     resp->reason);
    // Headers:
    struct header* header = resp->headers;
    while (header != NULL) {
        end += sprintf(str + end, "%s: %s\r\n", header->name,
            header->value);
        header = header->next;
    }
    end += sprintf(str + end, "\r\n");
    // Body:
    if (resp->body != NULL)
        end += sprintf(str + end, "%s", resp->body);
    return end;
}

void resp_free(struct response* resp) {
    free(resp->body);
    resp->body = NULL;
    
    struct header* runner = resp->headers;
    struct header* next;
    while (runner != NULL) {
        next = runner->next;
        free(runner->name);
        free(runner->value);
        free(runner);
        runner = next;
    }
    resp->headers = NULL;
}

