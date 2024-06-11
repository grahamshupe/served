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


int handle_get(struct response* resp, struct request* req, 
                const char* root_path) {
    int status = 200;
    FILE* file = NULL;

    // Try to get file:
    char* path = NULL;
    if (strcmp(req->target, "/") == 0) {
        path = malloc(16);
        strcpy(path, root_path);
        strcat(path, "/index.html");
    } else {
        path = malloc(strlen(req->target) + strlen(root_path));
        strcpy(path, root_path);
        strcat(path, req->target);
    }

    file = fopen(path, "r");
    if (file == NULL) {
        // Not found, so send the 404 file:
        free(path);
        path = malloc(14);
        strcpy(path, root_path);
        strcat(path, "/404.html");
        file = fopen(path, "r");
        status = 404;
    }

    // Get mime type:
    char mime[64];
    char* ext = strrchr(path, '.') + 1;
    if (strcmp(ext, "html") == 0)
        strcpy(mime, "text/html; charset=utf-8");
    else if (strcmp(ext, "js") == 0)
        strcpy(mime, "text/javascript");
    else if (strcmp(ext, "txt") == 0)
        strcpy(mime, "text/plain; charset=utf-8");
    else
        return 500;
    free(path);
    
    // Fill out the response:
    resp->body = malloc(BODY_SIZE);
    int size = fread(resp->body, 1, BODY_SIZE, file);
    resp->body[size] = '\0';
    fclose(file);

    char value[10];
    snprintf(value, 10, "%d", size);
    resp_add_header(resp, "Content-Length", value);
    resp_add_header(resp, "Content-Type", mime);

    return status;
}

int handle_post(struct response* resp, struct request* req) {
    FILE* dest = NULL;
    char* type = req_get_header(req, "content-type");

    if (type == NULL)
        return 400;
    if (strcmp(type, "application/x-www-form-urlencoded") == 0) {
        int length = atoi(req_get_header(req, "content-length")) - 7;
        char* answer = req->body + 7;
        dest = fopen("root/responses.txt", "a");
        fwrite(answer, 1, length, dest);
        fwrite("\n", 1, 1, dest);
        fclose(dest);
        resp_add_header(resp, "Location", "/responses.txt");

        dest = fopen("root/responses.txt", "r");
        resp->body = malloc(BODY_SIZE);
        int size = fread(resp->body, 1, BODY_SIZE, dest);
        resp->body[size] = '\0';
        fclose(dest);

        char value[10];
        snprintf(value, 10, "%d", size);
        resp_add_header(resp, "Content-Length", value);
        resp_add_header(resp, "Content-Type", "text/plain; charset=utf-8");

        return 303;
    } else {
        return 501;
    }
}

