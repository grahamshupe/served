#ifndef HANDLER_H_
#define HANDLER_H_

/*
Handles a read (EPOLLIN) event on the given conn_info fd.
Any pending requests will be parsed, and a response will be created.
This will attempt to send all pending responses, using handle_write_event().
Returns -1 on error, 0 if the socket has shut down, and 1 on success.
*/
int handle_read_event(conn_info_t* conn_info);

/*
Handles a write (EPOLLOUT) event on the given conn_info fd.
This constructs and sends a response to the pending request.
On success, the connection state is returned to IDLE, and req and resp are freed.
Returns -1 on error, 0 if the socket has shut down, and 1 on success.
*/
int handle_write_event(conn_info_t* conn_info);


#endif