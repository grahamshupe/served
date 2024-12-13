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


response_t* resp_new(int status) {
    response_t* resp = calloc(1, sizeof(response_t));
    strcpy(resp->protocol, "HTTP/1.1");
    resp_change_status(resp, status);

    // Add general headers:
    resp_add_header(resp, "Server", "Served");
    time_t timer = time(NULL);
    char* time = asctime(gmtime(&timer));
    time[24] = '\0';
    resp_add_header(resp, "Date", time);
    resp_add_header(resp, "Connection", "close");

    return resp;
}

void resp_change_status(response_t* resp, int status) {
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
        case 431:
            strcpy(resp->reason, "Request Header Fields Too Large");
            break;
        case 500:
            strcpy(resp->reason, "Internal Server Error");
            break;
        case 501:
            strcpy(resp->reason, "Not Implemented");
            break;
        case 505:
            strcpy(resp->reason, "HTTP Version Not Supported");
            break;

        default:
            strcpy(resp->reason, "");
            break;
    }
}

void resp_add_header(response_t* resp, const char* name, const char* value) {
    struct header* new = malloc(sizeof(struct header));

    new->name = malloc(strlen(name) + 1);
    strcpy(new->name, name);
    new->value = malloc(strlen(value) + 1);
    strcpy(new->value, value);

    new->next = resp->headers;
    resp->headers = new;
}

int resp_to_str(response_t* resp, char** str) {
    // Find the string length:
    int len = snprintf(NULL, 0, "%s %d %s\r\n", resp->protocol, resp->status, resp->reason);
    struct header* header = resp->headers;
    while (header != NULL) {
        len += strlen(header->name) + strlen(header->value) + 4;
        header = header->next;
    }
    len += 2; // account for last \r\n

    *str = malloc(len + 1);
    int end = sprintf(*str, "%s %d %s\r\n", resp->protocol, resp->status, resp->reason);
    header = resp->headers;
    while (header != NULL) {
        end += sprintf(*str + end, "%s: %s\r\n", header->name, header->value);
        header = header->next;
    }
    end += sprintf(*str + end, "\r\n");
    *str[len] = '\0';

    return len;
}

void resp_free(response_t* resp) {
    struct header* runner = resp->headers;
    struct header* next;
    while (runner != NULL) {
        next = runner->next;
        free(runner->name);
        free(runner->value);
        free(runner);
        runner = next;
    }
    free(resp);
}

char* resp_get_header(response_t* resp, const char* name) {
    struct header* runner = resp->headers;
    while (runner != NULL) {
        if (strcmp(runner->name, name) == 0)
            return runner->value;
        runner = runner->next;
    }
    return NULL;
}
