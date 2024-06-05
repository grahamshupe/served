/*
Functions for parsing and modifying HTTP responses and requests

TODO:
- _req_parse_headers() should recognize multiple field lines (RFC 9110:5.2)
- _req_parse helper functions should recognize both \r\n and \n as line endings
- make headers a hashset, instead of a linked list.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "parser.h"
#include "util.h"


/*
 * Private Functions *
*/

int _req_parse_start(char** message, size_t* size, struct request* req) {
    // Get method:
    if (*size < 16) {
        // startline can't possibly be long enough
        return 400;
    } else if (strncmp(*message, "GET ", 4) == 0) {
        req->method = GET;
        *message += 4;
        *size -= 4;
    } else if (strncmp(*message, "HEAD ", 5) == 0) {
        req->method = HEAD;
        *message += 5;
        *size -= 5;
    } else if (strncmp(*message, "POST ", 5) == 0) {
        req->method = POST;
        *message += 5;
        *size -= 5;
    } else {
        // invalid method
        return 501;
    }

    // Get target:
    size_t token_size = memcspn(*message, " ", *size);
    if (token_size == 0)
        return 400;  // no target or too many spaces given
    if (token_size == *size)
        return 414;  // target is too long

    req->target = malloc(token_size + 1);
    strncpy(req->target, *message, token_size);
    req->target[token_size] = '\0';
    *message += token_size + 1;
    *size -= token_size + 1;

    // Get protocol:
    if (*size < 10 || strncmp(*message, "HTTP/1.1\r\n", 10) != 0) {
        // invalid protocol (must be HTTP/1.1)
        free(req->target);
        req->target = NULL;
        return 400;
    }
    strncpy(req->protocol, "HTTP/1.1", 9);

    *message += 10;  // go to the end of the startline
    *size -= 10;
    return 200;
}

int _req_parse_headers(char** message, size_t* size, struct request* req) {
    size_t line_size = 0;
    req->headers = NULL;

    while ((line_size = memcspn(*message, "\r", *size)) != 0) {
        // Add header to the start of the headers linked list:
        struct header* header = malloc(sizeof(struct header));
        if (req->headers != NULL) {
            header->next = req->headers;
        } else {
            header->next = NULL;
        }
        req->headers = header;

        // Get field name:
        size_t name_size = memcspn(*message, ":", line_size);
        if (line_size == *size || name_size == line_size)
            return 400;
        header->name = malloc(name_size + 1);
        strncpy(header->name, *message, name_size);
        header->name[name_size] = '\0';
        str_tolower(header->name, name_size);
        *message += name_size + 1;

        // Get field value:
        size_t ows = strspn(*message, " ");
        *message += ows;  // ignore beginning optional white space
        size_t val_size = memcspn(
            *message, " \r", line_size - (name_size + 1 + ows));
        header->value = malloc(val_size + 1);
        strncpy(header->value, *message, val_size);
        header->value[val_size] = '\0';
        str_tolower(header->value, val_size);
        *message += line_size + 2 - (name_size + 1 + ows);
        *size -= line_size + 2;
    }

    *message += 2;
    *size -= 2;
    return 200;
}

/*
 * Public Functions *
*/

int req_parse(char* message, struct request* req, size_t msg_size) {
    req->headers = NULL;
    req->target = NULL;
    req->body = NULL;
    int status = 0;
    if ((status = _req_parse_start(&message, &msg_size, req)) != 200)
        return status;
    if ((status = _req_parse_headers(&message, &msg_size, req)) != 200)
        return status;

    // Parse body:
    req->body = malloc(msg_size + 1);
    strncpy(req->body, message, msg_size);
    req->body[msg_size] = '\0';

    return 200;
}

void req_free(struct request* req) {
    free(req->target);
    req->target = NULL;
    free(req->body);
    req->body = NULL;

    struct header* runner = req->headers;
    struct header* next;
    while (runner != NULL) {
        next = runner->next;
        free(runner->name);
        free(runner->value);
        free(runner);
        runner = next;
    }
}

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

char* req_get_header(struct request* req, const char* name) {
    struct header* runner = req->headers;
    while (runner != NULL) {
        if (strcmp(runner->name, name) == 0)
            return runner->value;
        runner = runner->next;
    }
    return NULL;
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
