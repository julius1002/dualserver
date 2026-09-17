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

struct dualserver {
	int port;
	struct {
		const char *path;
		size_t path_len;
		RequestHandler request_handler;
	} *handler_mappings;
	size_t num_mappings;
	size_t max_mappings;
};

void init_dualserver(struct dualserver *dserver, int port);

void add_handler_mapping(const char *path, void (*request_handler)(struct http_request *req, struct http_response *res), struct dualserver *dserver);

int launch_dualserver(struct dualserver *dserver);
