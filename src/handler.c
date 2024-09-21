/*
Functions for handling HTTP requests and gathering response data
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "request.h"
#include "response.h"
#include "handler.h"



void handle_connection(int client_fd, const char* root_path) {
    char* message = malloc(REQUEST_SIZE);
    char* msg_origin = message;  // keep track of start so we can free msg
    ssize_t received = recv(client_fd, message, REQUEST_SIZE, 0);
    if (received == -1) {
        perror("recv");
        free(message);
        return;
    }
    struct request req;
    int parse_status = req_parse(message, &req, received);
    struct response resp;
    resp_new(&resp);
    int resp_status = 500;
    if (parse_status != 200) {
        resp_status = parse_status;
    } else {
        switch (req.method) {
            case GET:
                resp_status = handle_get(&resp, &req, root_path);
                break;
            case POST:
                // right now, just deny all POST requests.
                // POST is functional, but since there is nothing to POST,
                // better to just deny it for security.
                resp_status = 403;
                //resp_status = handle_post(&resp, &req);
                break;
            case HEAD:
            default:
                resp_status = 501;
        }
    }
    resp_change_status(&resp, resp_status);
    char resp_msg[BODY_SIZE];
    int resp_size = resp_to_str(&resp, resp_msg);
    //printf("response:\n%s\n", resp_msg);
    int sent = send(client_fd, resp_msg, resp_size, 0);
    ZF_LOGI("Responded %d to connection attempt", resp.status);
    req_free(&req);
    resp_free(&resp);
    free(msg_origin);
    msg_origin, message = NULL;
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