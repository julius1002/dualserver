#include "picohttpparser-master/picohttpparser.h"
#include <unistd.h>
#include <string.h>

struct http_request {
	int conn;
	const char *method, *path;
	int minor_version; // HTTP 1
	size_t method_len, path_len, num_headers;
	struct phr_header headers[50];
	struct fd_set *master;
	int wakeup_fd;
};
