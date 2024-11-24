/*
Functions for parsing and modifying HTTP requests.

TODO:
- _req_parse_headers() should recognize multiple field lines (RFC 9110:5.2)
- _req_parse_start() should ignore any CRLF prior to request-line (RFC 9112:2.2)
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

static int _req_parse_start(request_t* req) {
    char* message = req->raw_message + req->bytes_read;
    // Get method:
    if (req->method == NULL) {
        if (req->msg_size < 6) {
            // startline can't possibly be long enough
            return 400;
        } else if (strncmp(message, "GET ", 4) == 0) {
            req->method = GET;
            message += 4;
            req->bytes_read += 4;
        } else if (strncmp(message, "HEAD ", 5) == 0) {
            req->method = HEAD;
            message += 5;
            req->bytes_read += 5;
        } else if (strncmp(message, "POST ", 5) == 0) {
            req->method = POST;
            message += 5;
            req->bytes_read += 5;
        } else {
            // invalid method
            return 501;
        }
    }

    // Get target:
    if (req->target == NULL) {
        ssize_t token_size = memcspn(message, " ", req->msg_size - req->bytes_read);
        if (token_size == 0) {
            // No target or too much white space
            return 400;
        } else if (token_size == -1) {
            if (req->msg_size - req->bytes_read != 0) {
                // Request line must not be formatted properly
                return 400;
            } else {
                // We reached the end of the message but we aren't done!
                return 0;
            }
        } else if (token_size > MAX_TARGET_SIZE) {
            // Target is too long
            return 414;
        }

        req->target = message;
        req->target[token_size] = '\0';
        message += token_size + 1;
        req->bytes_read += token_size + 1;
    }


    // Get protocol:
    if (req->protocol == NULL) {
        if (req->msg_size - req->bytes_read < 10 || strncmp(message, "HTTP/1.1\r\n", 10) != 0) {
            // invalid protocol (must be HTTP/1.1) or malformed line
            return 505;
        }

        req->protocol = message;
        req->protocol[8] = '\0';
        message += 10;  // go to the end of the startline
        req->bytes_read += 10;
    }

    return 200;
}

static int _req_parse_headers(request_t* req) {
    ssize_t line_size = 0;
    char* message = req->raw_message + req->bytes_read;

    while ((line_size = memcspn(message, "\r", req->msg_size)) != 0) {
        if (line_size == -1) {
            return 0;
        }

        struct header* header = malloc(sizeof(struct header));

        // Get field name:
        ssize_t name_size = memcspn(message, ":", line_size);
        if (name_size == -1 || name_size == line_size) {
            free(header);
            return 400;
        }
        header->name = message;
        header->name[name_size] = '\0';
        str_tolower(header->name, name_size);
        message += name_size + 1;

        // Get field value:
        ssize_t ows = memspn(message, " ", line_size - (name_size + 1));
        message += ows;  // ignore beginning optional white space
        ssize_t val_size = memcspn(message, " \r", line_size - (name_size + 1 + ows));
        if (val_size == -1) {
            free(header);
            return 400;
        }

        header->value = message;
        header->value[val_size] = '\0';
        str_tolower(header->value, val_size);
        message += line_size + 2 - (name_size + 1 + ows);
        req->bytes_read += line_size + 2;

        // Add header to the start of the headers linked list:
        if (req->headers != NULL) {
            header->next = req->headers;
        } else {
            header->next = NULL;
        }
        req->headers = header;
    }

    if (req->msg_size - req->bytes_read < 2 || message[0] != '\r' || message[1] != '\n') {
        return 400;
    }
    req->bytes_read += 2;
    return 200;
}

/*
 * Public Functions *
*/

request_t* req_parse(char* message, request_t* req, ssize_t msg_size, int* status) {
    if (req == NULL) {
        req = calloc(1, sizeof(request_t));
        req->raw_message = message;
        req->msg_size = msg_size;
    }

    *status = _req_parse_start(req);
    if (*status != 200) {
        return req;
    }

    *status = _req_parse_headers(req);
    if (*status != 200) {
        return req;
    }

    req->msg_size = req->bytes_read;
    req->raw_message = malloc(req->msg_size);
    strncpy(req->raw_message, message, req->msg_size);

    return 200;
}

void req_free(struct request* req) {
    free(req->target);
    req->target = NULL;
    free(req->body);
    req->body = NULL;
    free(req->raw_message);
    req->raw_message = NULL;

    struct header* runner = req->headers;
    struct header* next;
    while (runner != NULL) {
        next = runner->next;
        free(runner->name);
        free(runner->value);
        free(runner);
        runner = next;
    }

    free(req);
}

char* req_get_header(request_t* req, const char* name) {
    struct header* runner = req->headers;
    while (runner != NULL) {
        if (strcmp(runner->name, name) == 0)
            return runner->value;
        runner = runner->next;
    }
    return NULL;
}
