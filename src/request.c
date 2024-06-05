/*
Functions for parsing and modifying HTTP requests.

TODO:
- _req_parse_headers() should recognize multiple field lines (RFC 9110:5.2)
- _req_parse helper functions should recognize both \r\n and \n as line endings
- make headers a hashset, instead of a linked list.
*/

#include <stdlib.h>
#include <string.h>
#include "request.h"
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

char* req_get_header(struct request* req, const char* name) {
    struct header* runner = req->headers;
    while (runner != NULL) {
        if (strcmp(runner->name, name) == 0)
            return runner->value;
        runner = runner->next;
    }
    return NULL;
}
