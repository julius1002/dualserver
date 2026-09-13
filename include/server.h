#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include "threadpool.h"

#define PORT "3000"

#define RESPONSE "HTTP/1.0 404 NOT FOUND\r\nContent-Length: 9\r\n\r\nNot found"

struct dualserver {
	struct {
		char *path;
		size_t path_len;
		char * (*request_handler)(struct http_request *req);
	} *handler_mappings;
	size_t num_mappings;
	size_t max_mappings;
};

void init_dualserver(struct dualserver *dserver);
void add_handler_mapping(char *path, char * (*request_handler)(struct http_request *req), struct dualserver *dserver);
int launch_dualserver(struct dualserver *dserver);
