/*
Functions for handling HTTP requests and gathering response data
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/sendfile.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include "handler.h"
#include "request.h"
#include "response.h"
//#include "server.h"
#include "zf_log.h"

#define INTEGER_STRING_LEN 12
#define DEFAULT_BODY "<!DOCTYPE html><html><head><meta charset=\"UTF-8\"><title>Error</title></head><body>%s</body></html>"

static char* _get_file(char* target, response_t* resp, char* rootpath);
static char* _get_mime(const char* filename);

int handle_read_event(conn_info_t* conn_info, char* rootpath) {

    char* message = malloc(REQUEST_SIZE);
    ssize_t msg_size = conn_info->buffer_size;
    ssize_t received;
    int parse_status = -1;

    while ((received = recv(conn_info->fd, message + msg_size, REQUEST_SIZE - msg_size, 0)) > 0) {
        ssize_t msg_offset = 0;
        while (msg_offset < received) {
            msg_size = received - msg_offset;
            conn_info->req = req_parse(message + msg_offset, conn_info->req, msg_size, &parse_status);
            if (parse_status == 0) {
                memmove(message, message + msg_offset, msg_size);
                break;
            }

            ssize_t bytes_read = conn_info->req->bytes_read;
            msg_offset += bytes_read;

            conn_info->resp = resp_new(parse_status);
            handle_write_event(conn_info, rootpath);

            if (conn_info->state == WRITING) {
                if (msg_offset < received) {
                    // theres still another request waiting, so save it
                    conn_info->msg_buffer = memmove(message, message + msg_offset, msg_size);
                    conn_info->buffer_size = msg_size;
                } else {
                    free(message);
                }
                return 1;
            }

            req_free(conn_info->req);
            conn_info->req = NULL;
            resp_free(conn_info->resp);
            conn_info->resp = NULL;
        }
    }

    if (received == -1) {
        if (errno != EAGAIN && errno != EWOULDBLOCK) {
            // Unspecified error, panic
            conn_info->state = ERR;
            free(message);
            return -1;
        } else if (parse_status == 0) {
            // Theres still an unfinished request, save the remaining message
            conn_info->state = READING;
            conn_info->msg_buffer = message;
            conn_info->buffer_size = msg_size;
        } else {
            // No more requests on the socket
            conn_info->state = IDLE;
            free(message);
        }
        return 1;
    } else {
        // client has shut down the socket
        free(message);
        return 0;
    }
}

int handle_write_event(conn_info_t* conn_info, char* rootpath) {
    response_t* resp = conn_info->resp;
    request_t* req = conn_info->req;
    struct stat stat;
    if (conn_info->state != WRITING) {
        // Get body information:
        if (resp->status == 200 && (req->method == GET || req->method == HEAD)) {
            char* filename = _get_file(req->target, resp, rootpath);
            if (fstat(resp->body_fd, &stat) == -1) {
                ZF_LOGE("Could not retrieve information about file: error %d", errno);
                close(resp->body_fd);
                conn_info->state = ERR;
                return -1;
            }

            char filesize[INTEGER_STRING_LEN];
            snprintf(filesize, INTEGER_STRING_LEN, "%ld", stat.st_size);
            resp_add_header(resp, "Content-Length", filesize);
            resp_add_header(resp, "Content-Type", _get_mime(filename));
        }
    }

    // Send the status-line and headers (i.e. the message)
    if (resp->msg_size == 0 || resp->bytes_sent < resp->msg_size) {
        char* resp_str = NULL;
        int resp_len = resp_to_str(resp, &resp_str);
        resp->msg_size = resp_len;

        int sent;
        while (resp->bytes_sent < resp_len) {
            sent = send(conn_info->fd, resp_str + resp->bytes_sent, resp_len - resp->bytes_sent, 0);
            if (sent == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
                conn_info->state = WRITING;
                return 1;
            } else if (sent == -1) {
                conn_info->state = ERR;
                return -1;
            }
            resp->bytes_sent += sent;
        }
    }

    // Send the body:
    if (resp->body_fd > 0 && req->method != HEAD) {
        if (fstat(resp->body_fd, &stat) == -1) {
            ZF_LOGE("Could not retrieve information about file when sending body: error %d", errno);
            conn_info->state = ERR;
            return -1;
        }
        ssize_t sent;
        off_t offset = 0;
        while (resp->bytes_sent - resp->msg_size < stat.st_size) {
            sent = sendfile(conn_info->fd, resp->body_fd, &offset, stat.st_size - offset);
            if (sent == -1 && errno == EAGAIN) {
                conn_info->state = WRITING;
                return 1;
            } else if (sent == -1) {
                conn_info->state = ERR;
                return -1;
            }
            resp->bytes_sent += sent;
        }
    } else if (resp->body_fd == -1 && req->method != HEAD) {
        // Send a generic body message corresponding to the status reason
        int len = strlen(DEFAULT_BODY) + strlen(resp->reason) - 2;
        char* body = malloc(len);
        snprintf(body, len, DEFAULT_BODY, resp->reason);
        int sent = 0;

        while (sent < len) {
            errno = 0;
            sent += send(conn_info->fd, body + sent, len - sent, 0);
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                conn_info->state = WRITING;
                return 1;
            } else if (errno != 0) {
                conn_info->state = ERR;
                return -1;
            }
            resp->bytes_sent = sent;
        }
    }

    if (resp->body_fd > 0)
        close(resp->body_fd);

    conn_info->state = IDLE;
    req_free(req);
    conn_info->req = NULL;
    resp_free(resp);
    conn_info->resp = NULL;
    return 1;
}

/*
Opens the file specified by TARGET.
The body_fd field in RESP will be changed to the open file descriptor, or -1 if nonexistant.
On error, body_fd remains unchanged.
Returns the santized file path of TARGET, or null on error. If the file does not exist,
'404.html' will be returned and body_fd will be filled with it's file descriptor.
*/
static char* _get_file(char* target, response_t* resp, char* rootpath) {
    int fd;
    char* filename = target;

    if (filename[0] != '/') {
        ZF_LOGD("Client provided an invalid path: %s", filename);
        resp_change_status(resp, 400);
        return NULL;
    } else {
        filename++;
    }
    if (filename[0] == '\0')
        filename = "index.html";

    int root_fd = open(rootpath, O_RDONLY | __O_PATH | O_DIRECTORY);
    if (root_fd == -1) {
        ZF_LOGE("Could not open root path: error %d", errno);
        resp_change_status(resp, 500);
        return NULL;
    }

    fd = openat(root_fd, filename, O_RDONLY);

    if (fd == -1 && errno == EACCES) {
        resp_change_status(resp, 404);
        filename = "404.html";
        if ((fd = openat(root_fd, filename, O_RDONLY)) == -1) {
            close(root_fd);
            return NULL;
        }

    } else if (fd == -1) {
        ZF_LOGE("Could not open requested file: error %d", errno);
        resp_change_status(resp, 500);
        close(root_fd);
        return NULL;
    }

    close(root_fd);
    resp->body_fd = fd;
    return filename;
}

/*
Gets the mime type of the given FILENAME.
Returns a string representation of the mime type.
If FILENAME is null, the html mime type is returned.
*/
static char* _get_mime(const char* filename) {
    if (filename == NULL)
        return "text/html; charset=utf-8";
    char* ext = strrchr(filename, '.');
    if (ext == NULL)
        return "application/octet-stream";
    ext++;

    if (strcmp(ext, "html") == 0)
        return "text/html; charset=utf-8";
    else if (strcmp(ext, "js") == 0)
        return "text/javascript";
    else if (strcmp(ext, "txt") == 0)
        return "text/plain; charset=utf-8";
    else
        return "application/octet-stream";
}

