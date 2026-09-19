#include "../include/server.h"
#include <fcntl.h>

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
void parse_raw(char *buf, int nbytes, struct http_request *dst)
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
	for (size_t i = 0; i != dst->num_headers; ++i) {
	    printf("%.*s: %.*s\n", (int)dst->headers[i].name_len, dst->headers[i].name,
			    (int)dst->headers[i].value_len, dst->headers[i].value);
	}
        #endif
}

void prepare_signaling_pipe(int *fds) {
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
}

/*
 * Handle client data and hangups
 */
void handle_client_data(int fd, fd_set *master, int *fdmax)
{
	char buf[1024];    // buffer for client data
	int nbytes;

	if ((nbytes = recv(fd, buf, sizeof buf, 0)) <= 0) 
	{
		if (nbytes == 0) 
		{
			printf("selectserver: socket %d hung up\n", fd);
		} else {
			perror("recv");
		}
		close(fd);
		FD_CLR(fd, master);
	} 
	else 
	{
		int fds[2];
		prepare_signaling_pipe(fds);

		put_conn_by_pipe_fd(fd, fds[0]);

		FD_SET(fds[0], master);
		if (fds[0] > *fdmax) 
		{  
			// keep track of the max
			*fdmax = fds[0];
		}
		struct http_request *req = (struct http_request *) malloc(sizeof(struct http_request));
		con_to_req[fd] = req;
		req->wakeup_fd = fds[1];
		req->conn = fd;
		parse_raw(buf, nbytes, req);
		req->master = master;
		threadpool_enqueue(req);
	}
}

void notfound_handler(struct http_response *res) {
	res->status = 404;
	res->reason = "Not Found";
	res->body = "Not Found";
	res->body_len = 9;
        res->num_headers = 0;
}

void set_request_body(struct http_request *req) {
        char *content_len = NULL;
	for(size_t i = 0; i < req->num_headers; i++) {
		if(req->headers[i].name_len == 14 && strncmp(req->headers[i].name, "Content-Type", 14)) {
			content_len = malloc(sizeof(char) * req->headers[i].value_len + 1);
			memcpy(content_len, req->headers[i].value, req->headers[i].value_len);
		}
	}

	if(content_len != NULL) {
		struct phr_header *header_before_body = &req->headers[req->num_headers - 1];
		int clen = atoi(content_len);
		free(content_len);
		req->body = malloc(clen * sizeof(char) + 1);
		req->body_len = clen;
		memcpy((void *) req->body, ((char *)header_before_body->value) + 4 + header_before_body->value_len, clen);
	}
}

void handle_mapping(struct http_request *req, struct http_response *res, size_t *out_len, struct threadpool_arg *tp_arg, char **response, size_t i) {
	char *fname = NULL;
	enum webfile wf;
	if(req->path_len == 1 && req->path[0] == '/') {
		fname = "index.html";
		wf = HTML;
	} else {
	        fname = alloca(sizeof(char) * req->path_len);
		memcpy(fname, &req->path[1], req->path_len - 1);
		fname[req->path_len - 1] = '\0';
		wf = is_web_file(fname);
	}


	if(UNK != wf) {
                size_t file_len;
		char *file_contents = read_file(fname, tp_arg->dserver->static_files_loc, &file_len);
                if(file_contents != NULL) {
			init_static_files_response(res, file_contents, file_len, wf);
		} else {
			notfound_handler(res);
		}
		*response = serialize(res, out_len);
	} else if(req->path_len == tp_arg->handler_mappings[i].path_len 
			&& !strncmp(req->path, tp_arg->handler_mappings[i].path,
				tp_arg->handler_mappings[i].path_len)) {

		init_default_response(res);
		tp_arg->handler_mappings[i].request_handler(req, res);
		*response = serialize(res, out_len);
	
	}
}

void *thread_handle(void *arg) 
{
	while(1) 
	{
		struct entry *head = (struct entry *) threadpool_dequeue();
		struct http_request *req = head->data;
		struct threadpool_arg *tp_arg = (struct threadpool_arg *) arg;
		char *response = NULL;
		char *body = NULL;
		struct http_response *res = NULL;
		size_t out_len;

		set_request_body(req);
		res = malloc(sizeof(struct http_response));

		for(size_t i = 0; i < tp_arg->num_mappings; i++) {
			handle_mapping(req, res, &out_len, tp_arg, &response, i);
		}

		if(response == NULL) {
			notfound_handler(res);
			response = serialize(res, &out_len);
		}

	        if(0 > write(req->conn, response, out_len)) {
			perror("Error write");
			exit(1);
		}

		int wakeup_fd = req->wakeup_fd;
		char byte = '\0';
		if(0 > write(wakeup_fd, &byte, 1)) 
		{
			perror("write to wakeup pipe");
		}

		close(wakeup_fd);

		if(res != NULL) {
			free(res);
		}
	}
	return NULL;
}

int launch_dualserver(struct dualserver *dserver)
{
	struct threadpool_arg arg;
	arg.handler_mappings = dserver->handler_mappings;
	arg.num_mappings = dserver->num_mappings;
        arg.dserver = dserver;

	arg.thread_handle = thread_handle;
	init_threadpool(10, 1000, (void *) &arg);

	fd_set master, read_fds;
	int fdmax;
	int listener;

	FD_ZERO(&master);
	FD_ZERO(&read_fds);

	listener = tcp_listen(dserver->port, 10);

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
					 *
					 * we are also doing clean up here
					 */
					struct http_request *req = con_to_req[conn];
					close(conn);
					FD_CLR(conn, &master);
					close(i);
					FD_CLR(i, &master);
					remove_conn_by_pipe_fd(i);
					free((void *) req->body);
					free(req);
				} 
				else
				{
					handle_client_data(i, &master, &fdmax);
				}
			}
		}
	}
	return 0;
}

void add_handler_mapping(const char *path, RequestHandler request_handler, struct dualserver *dserver) {
	if(dserver->num_mappings >= dserver->max_mappings) {
		printf("handler not added, max_mappings reached\n"); // TODO replace with proper logging
	} else {
		dserver->handler_mappings[dserver->num_mappings].path = path;
		dserver->handler_mappings[dserver->num_mappings].path_len = strlen(path);
		dserver->handler_mappings[dserver->num_mappings++].request_handler = request_handler;
	}
}

void init_static_files(struct dualserver *dserver, const char *loc) {
	dserver->static_files_loc = loc;
}

void init_dualserver(struct dualserver *dserver, int port) {
	dserver->port = port;
	dserver->num_mappings = 0;
	dserver->max_mappings = 10;
	dserver->handler_mappings = malloc(dserver->max_mappings * sizeof(dserver->handler_mappings));
}
