#include "server.h"
#include "fcntl.h"

/**
 * Simple utility functions
 * This maps our pipe read end to a connection, if the read end is written to by a thread. 
 *
 * That means the thread wrote the response and the connection can be closed by the eventloop.
 * To signal this, the thread writes a byte to the pipe's write end, on which the event loop polls.
 */
int pipe_to_con[65536];

/* This maps a connection to a http request. We need it eventually to free the request in the event loop */
void *con_to_req[65536];

int find_conn_by_pipe_fd(int pipe_fd) {
	int ret;

	if((ret = pipe_to_con[pipe_fd]) == 0) {
		return -1;
	}

	return ret;
}

void put_conn_by_pipe_fd(int conn, int pipe_fd) {
	pipe_to_con[pipe_fd] = conn;
}

void remove_conn_by_pipe_fd(int pipe_fd) {
	pipe_to_con[pipe_fd] = 0;
}

int tcp_listen(uint16_t port, int backlog)
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1) 
    {
        return -1;
    }

    int one = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));

    struct sockaddr_in addr = {
	    .sin_family = AF_INET, 
	    .sin_port = htons(port), 
	    .sin_addr.s_addr = htonl(INADDR_ANY)
    };

    if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        return -1;
    }

    if (listen(fd, backlog) < 0) {
        return -1;
    }

    return fd;
}

/*
 * Add new incoming connections to the proper sets
 */
void handle_new_connection(int listener, fd_set *master, int *fdmax)
{
	socklen_t addrlen;
	int newfd;
	struct sockaddr_storage remoteaddr; // client address
	char remoteIP[INET6_ADDRSTRLEN];

	addrlen = sizeof remoteaddr;
	newfd = accept(listener, (struct sockaddr *)&remoteaddr, &addrlen);

	if (newfd < 0) {
		perror("accept");
	} else {
		FD_SET(newfd, master);
		if (newfd > *fdmax) {
			*fdmax = newfd;
		}
	}
}

/*
 * Parse data
 */
void parse_raw(char *buf, int nbytes, int listener, int s, fd_set *master, int fdmax, struct http_request *dst)
{
	size_t buflen = 0, prevbuflen = 0, method_len, path_len, num_headers;
	int pret;

        dst->num_headers = sizeof(dst->headers) / sizeof(dst->headers[0]);
	pret = phr_parse_request(buf, nbytes, &dst->method, &dst->method_len, &dst->path, &dst->path_len,
    		                             &dst->minor_version, dst->headers, &dst->num_headers, prevbuflen);

	if (pret > 0) {
                //break; /* successfully parsed the request */
	} else if (pret == -1) {
	        //return ParseError;
		return;
	/* request is incomplete, continue the loop */
	        assert(pret == -2);
	        if (buflen == sizeof(buf)) {
		//return RequestIsTooLongError;
			return;
	        }
	}

        #ifdef DEBUG
	printf("request is %d bytes long\n", pret);
	printf("method is %.*s\n", (int)dst->method_len, dst->method);
	printf("path is %.*s\n", (int)dst->path_len, dst->path);
	printf("HTTP version is 1.%d\n", dst->minor_version);
	printf("headers:\n");
	for (int i = 0; i != dst->num_headers; ++i) {
	    printf("%.*s: %.*s\n", (int)dst->headers[i].name_len, dst->headers[i].name,
			    (int)dst->headers[i].value_len, dst->headers[i].value);
	}
        #endif

	/*
	for(int j = 0; j <= fdmax; j++) {
		// send to everyone!
		if (FD_ISSET(j, master)) {
			// except the listener and ourselves
			if (j != listener && j != s) {
				if (send(j, buf, nbytes, 0) == -1) {
					perror("send");
				}
			}
		}
	}*/
}
/*
 * Handle client data and hangups
 */
void handle_client_data(int fd, int listener, fd_set *master, int *fdmax)
{
	char buf[512];    // buffer for client data
	int nbytes;

	// handle data from a client
	if ((nbytes = recv(fd, buf, sizeof buf, 0)) <= 0) 
	{
		// got error or connection closed by client
		if (nbytes == 0) 
		{
			// connection closed
			printf("selectserver: socket %d hung up\n", fd);
		} else {
			perror("recv");
		}
		close(fd); // bye!
		FD_CLR(fd, master); // remove from master set
	} 
	else 
	{
		int fds[2];
		if(pipe(fds) < 0) 
		{
			perror("pipe");
			exit(1);
		}

		int read_fd = fds[0];
		int flags = fcntl(read_fd, F_GETFL);

		// this needs to be nonblocking for the eventloop
		// the write fd stays blocking, since our blocking thread writes to it
		fcntl(read_fd, F_SETFL, flags | O_NONBLOCK);

		put_conn_by_pipe_fd(fd, read_fd);

		FD_SET(read_fd, master);
		if (read_fd > *fdmax) 
		{  
			// keep track of the max
			*fdmax = read_fd;
		}
		struct http_request *req = (struct http_request *) malloc(sizeof(struct http_request));
		con_to_req[fd] = req;
		req->wakeup_fd = fds[1];
		req->conn = fd;
		parse_raw(buf, nbytes, listener, fd, master, *fdmax, req);
		req->master = master;
		threadpool_enqueue(req);
	}
}

void *consume(void *arg) 
{
	while(1) 
	{
		struct entry *head = (struct entry *) threadpool_dequeue();
		struct http_request *req = head->data;
	        write(req->conn, RESPONSE, strlen(RESPONSE));

		int wakeup_fd = req->wakeup_fd;
		char byte = '\0';
		if(0 > write(wakeup_fd, &byte, 1)) 
		{
			perror("write to wakeup pipe");
		}
		close(wakeup_fd);
	}
	return NULL;
}

/*
 * Main
 */
int main(void)
{
	init_threadpool(10, 1000, consume);

	fd_set master, read_fds;
	int fdmax;
	int listener;

	FD_ZERO(&master);
	FD_ZERO(&read_fds);

	listener = tcp_listen(3000, 10);

	FD_SET(listener, &master);

	fdmax = listener;

	for(;;) {
		read_fds = master;
		if (select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1) {
			perror("select");
			exit(1);
		}

		for(int i = 0; i <= fdmax; i++) {
			if (FD_ISSET(i, &read_fds)) { // we got one!!
	                        int conn;
				if (i == listener) 
				{ 
					handle_new_connection(i, &master, &fdmax);
				}
				else if((conn = find_conn_by_pipe_fd(i)) > 0) 
				{       
					/* here we are getting the signal from the thread,
					 * that the event loop can close the connection safely
					 */
					close(conn);
					FD_CLR(conn, &master);
					close(i);
					FD_CLR(i, &master);
					remove_conn_by_pipe_fd(i);
					free(con_to_req[conn]);
				} 
				else
				{
					handle_client_data(i, listener, &master, &fdmax);
				}
			}
		}
	}
	return 0;
}
