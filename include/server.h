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
	struct handler_mapping *handler_mappings;
	size_t num_mappings;
	size_t max_mappings;
	char *static_files_loc;
};

void init_dualserver(struct dualserver *dserver, int port);

void add_handler_mapping(const char *path, RequestHandler request_handler, struct dualserver *dserver);

int launch_dualserver(struct dualserver *dserver);

void init_static_files(struct dualserver *dserver, char *loc);
