#ifndef HANDLER_H_
#define HANDLER_H_

/*
Handles a read (EPOLLIN) event on the given conn_info fd.
*/
void handle_connection(conn_info_t* conn_info, const server_opts_t* opts);

/*
Fill the given RESP as a response to a http GET request.
*/
int handle_get(struct response* resp, struct request* req, 
                const char* root_path);

/*
Fill the given RESP as a response to a http POST request.
*/
int handle_post(struct response* resp, struct request* req);

#endif